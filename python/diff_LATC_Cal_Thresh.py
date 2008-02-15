#! /usr/bin/env python
"""
Diff 2 LATC Cal_Thresh XML files.

Provide library class for utilization by other python scripts and main routine
for simple commandline execution.

The commandline is:
diff_LATC_Cal_Thresh.py xml_reference new_xml output_basefilename

where:
     xml_reference       - reference settings for comparison
     new_xml             - new settings
     output_basefilename - output files are based on this name with added extentions

    
"""

__facility__    = "Offline"
__abstract__    = "Diff 2 LATC Cal_Thresh XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/12 15:18:01 $"
__version__     = "$Revision: 1.6 $, $Author: fewtrell $"
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

    # create ttree target values (double arrays of length 1)
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

def genScatterDiff(ref_data, new_data, dacType, refName, newName):
    title = dacType+"_scatter"
    s = ROOT.TH2S(title,
                  title,
                  128,0,128,
                  128,0,128)

    s.SetXTitle(refName)
    s.SetYTitle(newName)
    s.SetMarkerStyle(ROOT.kFullTriangleUp)
    s.SetMarkerSize(2)

    import numarray
    for x_ref, x_new in zip(numarray.ravel(ref_data), numarray.ravel(new_data)):
        s.Fill(x_ref, x_new)

    s.Write()

def genDiffHist(ref_data, new_data, dacType, refName, newName):
    import ROOT

    title = dacType + "_diff"
    
    h = ROOT.TH1S(title,title,128,0,0)

    h.SetXTitle(newName + " - " + refName)

    diff = new_data - ref_data

    import numarray
    for x in numarray.ravel(diff):
        h.Fill(x)

    h.Write()

def genDiffPlots(ref_data, new_data, dacType, refName, newName):
    genScatterDiff(ref_data, new_data, dacType, refName, newName)
    genDiffHist(ref_data, new_data, dacType, refName, newName)

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


def findOutliers(ref_data, new_data, threshold):
    diff = new_data - ref_data

    import numarray
    diff = numarray.fabs(diff)

    outliers = diff > threshold

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


def genLACDiffSummary(refData,
                      newData):
    """
    generate diff plots specific to log_acpt CAL register
    """
    title = "log_acpt diff per GCCC"
    h = ROOT.TH2S(title,
                  title,
                  4,0,4,
                  256,-128,128)

    h.SetXTitle("GCCC")
    h.SetYTitle("log_acpt_diff")
    h.SetMarkerStyle(ROOT.kFullTriangleUp)
    h.SetMarkerSize(2)

    diff = newData - refData

    # plot diff by ccc
    import numarray
    import calDacXML
    for ccc in range(4):
        for crc in range(4):
            (row, end) = calDacXML.ccToRow(ccc,crc)
            for x in numarray.ravel(diff[:,row,end,:]):
                h.Fill(ccc,x)

    h.Write()

def genCFEPrecinctDiffSummary(refPath,
                              newPath,
                              dacType):
    import calDacXML
    import numarray

    # check that precinct data is present in both old & new files
    if not xml_file_contains_tag(refPath, dacType):
        return
    print dacType + " precinct data found in " + refPath
    if not xml_file_contains_tag(newPath, dacType):
        return
    print dacType + " precinct data found in " + newPath

    # get base filenames for plot & axis titles, etc...
    refName = basename(refPath)
    newName = basename(newPath)

    # get reference settings (convert to signed int so I can get correct diffs)
    try:
        ref = calDacXML.calSettingsXML(refPath, dacType).read().astype(numarray.Int8)
    except:
        ref = calDacXML.calDacXML(refPath, dacType).read().astype(numarray.Int8)

    # generate tuples based on reference settings
    genCalDacTuple(ref, refName+"_"+dacType)

    # get new settings (convert to signed int so I can get correct diffs)
    new = calDacXML.calSettingsXML(newPath, dacType).read().astype(numarray.Int8)

    # create tuples from new settings
    genCalDacTuple(new, newName+"_"+dacType)

    # generate hitograms of new - ref differences
    genDiffPlots(ref, new, dacType, refName, newName)

    # print mean diff report
    import numarray
    diff = new - ref
    meanDiff = numarray.average(numarray.ravel(diff))

    # mlab is part of numarray
    import numarray.mlab
    meanRMS = numarray.mlab.std(numarray.ravel(diff))

    # print mean diff report
    print "\nDAC\tmeanDiff\trms (DAC units)"
    print "%s\t%.2f\t%.2f"%(dacType, meanDiff, meanRMS)

    # find outliers
    OUTLIER_N_RMS = 3
    outliers = findOutliers(ref, new, meanRMS*OUTLIER_N_RMS)

    # print outlier report
    if len(outliers) > 0:
        print "\n%s outliers (> %d RMS)"%(dacType, OUTLIER_N_RMS)
        print "GTEM\tGCCC\tGCRC\tGCFE\tREF\tNEW\tDIFF"
        for idx in outliers:
            (tem,ccc,rc,fe) = idx
            import calDacXML
            (row, end) = calDacXML.ccToRow(ccc,rc)

            ref_val = ref[tem,row,end,fe]
            new_val = new[tem,row,end,fe]
            print "%d\t%d\t%d\t%d\t%d\t%d\t%d"%(tem,ccc,rc,fe,ref_val,new_val,new_val-ref_val)

    # print special reports
    if dacType == "log_acpt":
        genLACDiffSummary(ref,new)


def genCFEDiffSummary(refPath,
                      newPath):
    # generate summary for each of 4 precincts in CFE LATC component
    genCFEPrecinctDiffSummary(refPath, newPath, "log_acpt")
    genCFEPrecinctDiffSummary(refPath, newPath, "fle_dac")
    genCFEPrecinctDiffSummary(refPath, newPath, "fhe_dac")
    genCFEPrecinctDiffSummary(refPath, newPath, "rng_uld_dac")


if __name__ == '__main__':

    # check command line
    import sys
    args = sys.argv[1:]
    if len(args) != 3:
        print __doc__
        sys.exit(1)


    # get filenames
    (refPath, newPath, basePath) = args

    rootPath = basePath+".root"


    # open ROOT file
    import ROOT
    print "Opening " + rootPath + " for write"
    rootFile = ROOT.TFile(rootPath, "RECREATE")

    # call library method to do the real work.
    genCFEDiffSummary(refPath, newPath)

    # close ROOT file
    rootFile.Write()
    rootFile.Close()


    
