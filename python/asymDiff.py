"""
Diff 2 CAL asymmetry (asymmetry) offline calibration XML files.  The command line is:

python asymDiff.py <asym_xml_file1> <asym_xml_file2> <output_root_file>

where:
    <asym_xml_file1>    = GLAST Cal asymmetry offline calib file
    <asym_xml_file2>    = GLAST Cal asymmetry offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL asymmetry XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/11 21:35:57 $"
__version__     = "$Revision: 1.7 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calCalibXML
import calConstant
import logging
import cgc_util
import array
import numarray

# setup logger

logging.basicConfig()
log = logging.getLogger('asymDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(__doc__)
    sys.exit(1)


# get filenames
asymPath1 = sys.argv[1]
asymPath2 = sys.argv[2]
rootPath = sys.argv[3]

# read in dac xml files
log.info("Opening %s"%asymPath1)
asymFile1 = calCalibXML.calAsymCalibXML(asymPath1)
log.info("Opening %s"%asymPath2)
asymFile2 = calCalibXML.calAsymCalibXML(asymPath2)

# check that towers are the same (ok if they are, just print warning)
asymTwrs1 = asymFile1.getTowers()
asymTwrs2 = asymFile2.getTowers()

if (asymTwrs1 != asymTwrs2):
    log.error("input files have different n towers.  I quit! ;)")
    sys.exit(-1)


# load up arrays
log.info("Reading %s"%asymPath1)
(xpos1, asym1) = asymFile1.read()
log.info("Reading %s"%asymPath2)
(xpos2, asym2) = asymFile2.read()
# check xpos data
if (xpos1 != xpos2):
    log.error("xpos data do not match.  I quit!")

#convert to stanard python arrays (used by ROOT)
length = len(xpos1)
xpos = array.array('f',xpos1)
# just using zero for xerror as it is not measured
xerror = array.array('f',numarray.zeros(10,numarray.Float32))

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "asymDiff(%s,%s)"%(asymPath1,asymPath2),
                      9)

# gobal summary histogram
# indexed by (pdiode,ndiode) tuple
diff_summary = {}
err_diff_summary = {}

import os
asymFile1 = os.path.basename(asymPath1)
asymFile2 = os.path.basename(asymPath2)

for pdiode in range(2):
    for ndiode in range(2):
        diode_tpl = (pdiode,ndiode)
        diff_summary[diode_tpl] = ROOT.TH1I("diff_summary_%d%d"%diode_tpl,
                                            "diff_summary_%d%d %s-%s"%(pdiode,ndiode,asymFile2,asymFile1),
                                            100,0,0)
        err_diff_summary[diode_tpl] = ROOT.TH1I("err_diff_summary_%d%d"%diode_tpl,
                                                "err_diff_summary_%d%d %s-%s"%(pdiode,ndiode,asymFile2,asymFile1),
                                                100,0,0)


# calc diffs for each channel
for twr in asymTwrs1:
    log.info("Processing TEM=%d"%twr)
    for lyr in range(calConstant.NUM_LAYER):
        # calCalibXML uses 'row' indexing, not layer
        row = calCalibXML.layerToRow(lyr)
        for col in range(calConstant.NUM_FE):
            # diode on positive face
            for pdiode in range(2):
                for ndiode in range(2):
                    diode_tpl = (pdiode,ndiode)

                    channel_str = "%d_%d_%d_%d_%d"%(twr,lyr,col,pdiode,ndiode)

                    ## CALC INDICES ##
                    asymIdx = cgc_util.asymIdx[(pdiode,ndiode,False)]
                    errIdx  = cgc_util.asymIdx[(pdiode,ndiode,True)]

                    ## GENERATE ARRAYS FOR THIS CHANNEL ##
                    channel1 = asym1[twr,row,col,asymIdx]
                    error1   = asym1[twr,row,col,errIdx]
                    channel2 = asym2[twr,row,col,asymIdx]
                    error2   = asym2[twr,row,col,errIdx]
                    asymDiff = channel2 - channel1
                    errDiff  = error2 - error1

                    # convert to standard arrays
                    asymDiff = array.array('f',asymDiff)
                    errDiff  = array.array('f',errDiff)

                    channel1  = array.array('f',channel1)
                    channel2  = array.array('f',channel2)
                    error1  = array.array('f',error1)
                    error2  = array.array('f',error2)

                    ## FILL HISTS ##
                    for asym in asymDiff:
                        diff_summary[diode_tpl].Fill(asym)

                    for err in errDiff:
                        err_diff_summary[diode_tpl].Fill(err)

                    ## GENERATE GRAPHS ##
                    # asym diffs
                    gAsym = ROOT.TGraph(length, xpos, asymDiff)
                    gAsym.SetNameTitle("asymDiff_%s"%channel_str,
                                       "asymDiff_%s %s-%s"%(channel_str,asymFile2,asymFile1))
                    cAsym = ROOT.TCanvas("asymDiff_%s"%channel_str,
                                         "asymDiff_%s %s-%s"%(channel_str,asymFile2,asymFile1),-1)
                    gAsym.Draw("AL*")
                    cAsym.Write()

                    # overlay plots
                    mg = ROOT.TMultiGraph("overlay_%s"%channel_str,
                                          "overlay_%s"%channel_str)
                    leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)
                    gOld = ROOT.TGraphErrors(length,xpos, channel1,
                                             xerror, error1)
                    gOld.SetMarkerColor(1) # black
                    leg.AddEntry(gOld, asymFile1,"P")
                    gNew = ROOT.TGraphErrors(length,xpos, channel2,
                                             xerror, error2)
                    gNew.SetMarkerColor(2) # red
                    leg.AddEntry(gNew, asymFile2,"P")
                    mg.Add(gOld,"l*")
                    mg.Add(gNew,"l*")


                    cOlay = ROOT.TCanvas("overlay_%s"%channel_str,
                                         "overlay_%s"%channel_str,
                                         -1)
                    mg.Draw("a")
                    leg.Draw()
                    cOlay.Write()

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
