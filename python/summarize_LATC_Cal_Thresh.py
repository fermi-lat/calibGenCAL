#! /usr/bin/env python
"""
Summarize LATC Cal_Thresh XML files.

Provide library class for utilization by other python scripts and main routine
for simple commandline execution.

The commandline is:
summarize_LATC_Cal_Thresh.py xml output_basefilename

where:
     xml             - LATC Cal threshold settings
     output_basefilename - output files are based on this name with added extentions

    
"""

__facility__    = "Offline"
__abstract__    = "Summarize LATC Cal_Thresh XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/12 15:18:01 $"
__version__     = "$Revision: 1.4 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


"""
generate ROOT TTree object (aka nTuple) for given Cal Dac discriminator settings

Param: array [16,8,2,12] of Cal dac settings

Returns: ROOT TTree object with following column definitions:
GTEM - tower module (0-15)
GCCC - cable controller (0-4)
GCRC - row controller (0-4)
GCFE - front-end controller (0-12)
DAC  - discriminator DAC setting (0-127)

"""
def genCalDacTuple(data, title):
    import ROOT
    t = ROOT.TTree(title, title)

    # create ttree target valeus (double arrays of length 1)
    import array
    _tem = array.array('H', [0])
    _ccc = array.array('H', [0])
    _rc = array.array('H', [0])
    _fe = array.array('H', [0])
    _dac = array.array('H', [0])

    # create ttree branches
    t.Branch("GTEM", _tem, "GTEM/s")
    t.Branch("GCCC", _ccc, "GCCC/s")
    t.Branch("GCRC", _rc, "GCRC/s")
    t.Branch("GCFE", _fe, "GCFE/s")
    t.Branch("DAC", _dac, "DAC/s")

    import calConstant
    import calDacXML
    for tem in range(calConstant.NUM_TEM):
        for ccc in range(calConstant.NUM_GCCC):
            for rc in range(calConstant.NUM_GCRC):
                for fe in range(calConstant.NUM_FE):
                    (row, end) = calDacXML.ccToRow(ccc,rc)

                    # populate tuple parms
                    _tem[0] = tem
                    _ccc[0] = ccc
                    _rc[0] = rc
                    _fe[0] = fe
                    _dac[0] = data[tem][row][end][fe]

                    t.Fill()

    t.Write()

def genSummaryHist(data, dacType, name):
    import ROOT

    title = dacType + "_summary"
    
    h = ROOT.TH1S(title,title,128,0,0)

    h.SetXTitle(name)

    import numarray
    for x in numarray.ravel(data):
        h.Fill(x)

    h.Write()

"""
given Cal Dac setting shape array with true & false values
return list of index tuples for each true value
"""
def findIndices(data):
    retList = []

    import calConstant
    import calDacXML
    for tem in range(calConstant.NUM_TEM):
        for ccc in range(calConstant.NUM_GCCC):
            for rc in range(calConstant.NUM_GCRC):
                for fe in range(calConstant.NUM_FE):
                    (row, end) = calDacXML.ccToRow(ccc,rc)

                    if data[tem][row][end][fe]:
                        retList.append((tem,ccc,rc,fe))

    return retList


def findOutliers(data, mean, threshold):
    import numarray
    data = numarray.fabs(data)

    outliers = data - mean > threshold

    return findIndices(outliers)

def xml_file_contains_tag(path, tag):
    import Ft

    uri = Ft.Lib.Uri.OsPathToUri(path)
    reader = Ft.Xml.Domlette.NonvalidatingReader
    doc = reader.parseUri(uri)

    return len(doc.xpath('.//%s'%tag)) > 0

def basename(path):
    """
    trim directory prefix & extension off of filename
    """
    import os
    return os.path.splitext(os.path.split(path)[1])[0]


def ccc_plot(data, name, dacType):
    """
    scatter plot settings by GCCC
    """
    h = ROOT.TH2S(name,
                  name,
                  4,0,4,
                  256,-128,128)

    h.SetXTitle("GCCC")
    h.SetYTitle(dacType)
    h.SetMarkerStyle(ROOT.kFullTriangleUp)
    h.SetMarkerSize(2)

    # plot by ccc
    import numarray
    import calDacXML
    for ccc in range(4):
        for crc in range(4):
            (row, end) = calDacXML.ccToRow(ccc,crc)
            for x in numarray.ravel(data[:,row,end,:]):
                h.Fill(ccc,x)

    h.Write()

def genCFEPrecinctSummary(path,
                          dacType):
    import calDacXML
    import numarray

    # check that precinct data is present in both old & new files
    if not xml_file_contains_tag(path, dacType):
        return
    print dacType + " precinct data found in " + path

    # get base filenames for plot & axis titles, etc...
    name = basename(path)

    # get dac settings 
    data = calDacXML.calSettingsXML(path, dacType).read()

    # generate tuples based on settings
    genCalDacTuple(data, name+"_"+dacType)

    # generate summary histograms 
    genSummaryHist(data, dacType, name)

    # print mean value report
    import numarray
    mean = numarray.average(numarray.ravel(data))

    # mlab is part of numarray
    import numarray.mlab
    rms = numarray.mlab.std(numarray.ravel(data))

    # print mean 
    print "\nDAC\tmean\trms (DAC units)"
    print "%s\t%.2f\t%.2f"%(dacType, mean, rms)

    # find outliers
    OUTLIER_N_RMS = 3
    outliers = findOutliers(data, mean, rms*OUTLIER_N_RMS)

    # print outlier report
    if len(outliers) > 0:
        print "\n%s outliers (> %d RMS)"%(dacType, OUTLIER_N_RMS)
        print "GTEM\tGCCC\tGCRC\tGCFE\tDAC"
        for idx in outliers:
            (tem,ccc,rc,fe) = idx
            import calDacXML
            (row, end) = calDacXML.ccToRow(ccc,rc)

            val = data[tem,row,end,fe]
            print "%d\t%d\t%d\t%d\t%d"%(tem,ccc,rc,fe,val)

    # print special reports
    if dacType == "log_acpt":
        ccc_plot(data, name, dacType)


def genCFESummary(path):
    # generate summary for each of 4 precincts in CFE LATC component
    genCFEPrecinctSummary(path, "log_acpt")
    genCFEPrecinctSummary(path, "fle_dac")
    genCFEPrecinctSummary(path, "fhe_dac")
    genCFEPrecinctSummary(path, "rng_uld_dac")


if __name__ == '__main__':

    # check command line
    import sys
    args = sys.argv[1:]
    if len(args) != 2:
        print __doc__
        sys.exit(1)


    # get filenames
    (path, basePath) = args

    rootPath = basePath+".root"


    # open ROOT file
    import ROOT
    print "Opening " + rootPath + " for write"
    rootFile = ROOT.TFile(rootPath, "RECREATE")

    # call library method to do the real work.
    genCFESummary(path)

    # close ROOT file
    rootFile.Write()
    rootFile.Close()


    
