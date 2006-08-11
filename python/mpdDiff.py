"""
Diff 2 CAL mevPerDAC (mevPerDAC) offline calibration XML files.  The command line is:

python mpdDiff.py <mpd_xml_file1> <mpd_xml_file2> <output_root_file>

where:
    <mpd_xml_file1>    = GLAST Cal mevPerDAC offline calib file
    <mpd_xml_file2>    = GLAST Cal mevPerDAC offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL mevPerDAC XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2006/08/10 18:06:43 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import logging
import array
import Numeric


import calCalibXML
import calConstant
import zachUtil

usage = "Usage: python mpdDiff.py <mpd_xml_file1> <mpd_xml_file2> <output_root_file>"

# setup logger

logging.basicConfig()
log = logging.getLogger('mpdDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(usage)
    sys.exit(1)


# get filenames
mpdPath1 = sys.argv[1]
mpdPath2 = sys.argv[2]
rootPath = sys.argv[3]

# read in dac xml files
log.info("Opening %s"%mpdPath1)
mpdFile1 = calCalibXML.calMevPerDacCalibXML(mpdPath1)
log.info("Opening %s"%mpdPath2)
mpdFile2 = calCalibXML.calMevPerDacCalibXML(mpdPath2)

# check that towers are the same (ok if they are, just print warning)
mpdTwrs1 = mpdFile1.getTowers()
mpdTwrs2 = mpdFile2.getTowers()

if (mpdTwrs1 != mpdTwrs2):
    log.error("input files have different n towers.  I quit! ;)")


# load up arrays
log.info("Reading %s"%mpdPath1)
mpd1 = mpdFile1.read()
log.info("Reading %s"%mpdPath2)
mpd2 = mpdFile2.read()

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "mpdDiff(%s,%s)"%(mpdPath1,mpdPath2))

# gobal summary histogram
lrg_diff_summary = ROOT.TH1I("lrg_diff_summary",
                             "lrg_diff_summary",
                             100,0,0)
lrg_err_diff_summary = ROOT.TH1I("lrg_err_diff_summary",
                                 "lrg_err_diff_summary",
                                 100,0,0)
sm_diff_summary = ROOT.TH1I("sm_diff_summary",
                            "sm_diff_summary",
                            100,0,0)
sm_err_diff_summary = ROOT.TH1I("sm_err_diff_summary",
                                "sm_err_diff_summary",
                                100,0,0)


mpdDiff = mpd2 - mpd1

for twr in mpdTwrs1:
    lrgDiff    = mpdDiff[twr, ..., zachUtil.mpdBigValIdx]
    smDiff     = mpdDiff[twr, ..., zachUtil.mpdSmallValIdx]
    lrgErrDiff = mpdDiff[twr, ..., zachUtil.mpdBigSigIdx]
    smErrDiff  = mpdDiff[twr, ..., zachUtil.mpdSmallSigIdx]

    for lrg in Numeric.ravel(lrgDiff):
        lrg_diff_summary.Fill(lrg)
    for lErr in Numeric.ravel(lrgErrDiff):
        lrg_err_diff_summary.Fill(lErr)
    for sm in Numeric.ravel(smDiff):
        sm_diff_summary.Fill(sm)
    for smErr in Numeric.ravel(smErrDiff):
        sm_err_diff_summary.Fill(smErr)

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()

















