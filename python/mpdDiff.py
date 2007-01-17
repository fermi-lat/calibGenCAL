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
__date__        = "$Date: 2007/01/08 19:45:01 $"
__version__     = "$Revision: 1.4 $, $Author: fewtrell $"
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

# calc diffs
mpdDiff = mpd2 - mpd1

#boolean map of active channels
channelMap = mpd1.copy()
channelMap[:] = 0
for twr in mpdTwrs1:
    channelMap[twr,:] = 1

# determine data limits for all plots
minVals = None
maxVals = None
minDiff = None
maxDiff = None

for twr in mpdTwrs1:
    for idx in range(zachUtil.N_MPD_IDX):
        # first pass only compare against 1st set
        if minVals is None:
            minVals = Numeric.ones(zachUtil.N_MPD_IDX,'f')*1e5
            maxVals = Numeric.ones(zachUtil.N_MPD_IDX,'f')*-1e5
            minDiff = Numeric.ones(zachUtil.N_MPD_IDX,'f')*1e5
            maxDiff = Numeric.ones(zachUtil.N_MPD_IDX,'f')*-1e5

            minVals[idx] = min(Numeric.ravel(mpd1[twr, ..., idx]))
            maxVals[idx] = max(Numeric.ravel(mpd1[twr, ..., idx]))
            minVals[idx] = min(minVals[idx], min(Numeric.ravel(mpd2[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(Numeric.ravel(mpd2[twr, ..., idx])))

            minDiff[idx] = min(Numeric.ravel(mpdDiff[twr, ..., idx]))
            maxDiff[idx] = max(Numeric.ravel(mpdDiff[twr, ..., idx]))

        else:
            minVals[idx] = min(minVals[idx], min(Numeric.ravel(mpd1[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(Numeric.ravel(mpd1[twr, ..., idx])))
            minVals[idx] = min(minVals[idx], min(Numeric.ravel(mpd2[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(Numeric.ravel(mpd2[twr, ..., idx])))

            minDiff[idx] = min(minDiff[idx], min(Numeric.ravel(mpdDiff[twr, ..., idx])))
            maxDiff[idx] = max(maxDiff[idx], max(Numeric.ravel(mpdDiff[twr, ..., idx])))


# gobal summary histograms
lrg_diff_summary = ROOT.TH1I("lrg_diff_summary",
                             "lrg_diff_summary (mpd2 - mpd1)",
                             50, minDiff[zachUtil.mpdBigValIdx], maxDiff[zachUtil.mpdBigValIdx])


lrg_err_diff_summary = ROOT.TH1I("lrg_err_diff_summary",
                                 "lrg_err_diff_summary (mpd2 - mpd)",
                                 50, minDiff[zachUtil.mpdBigSigIdx], maxDiff[zachUtil.mpdBigSigIdx])


sm_diff_summary = ROOT.TH1I("sm_diff_summary",
                            "sm_diff_summary (mpd2 - mpd1)",
                            50, minDiff[zachUtil.mpdSmallValIdx], maxDiff[zachUtil.mpdSmallValIdx])


sm_err_diff_summary = ROOT.TH1I("sm_err_diff_summary",
                                "sm_err_diff_summary (mpd2 - mpd1)",
                                50, minDiff[zachUtil.mpdSmallSigIdx], maxDiff[zachUtil.mpdSmallSigIdx])



# build scatter histograms

lrg_scatter = ROOT.TH2S("lrg_scatter",
                        "lrg diode mpd change x=mpd1 y=mpd2",
                        100, minVals[zachUtil.mpdBigValIdx], maxVals[zachUtil.mpdBigValIdx],
                        100, minVals[zachUtil.mpdBigValIdx], maxVals[zachUtil.mpdBigValIdx])

sm_scatter = ROOT.TH2S("sm_scatter",
                       "sm diode mpd change x=mpd1 y=mpd2",
                       100, minVals[zachUtil.mpdSmallValIdx], maxVals[zachUtil.mpdSmallValIdx],
                       100, minVals[zachUtil.mpdSmallValIdx], maxVals[zachUtil.mpdSmallValIdx])

lrg_err_scatter = ROOT.TH2S("lrg_err_scatter",
                            "lrg diode mpd error change x=mpd1 y=mpd2",
                            100, minVals[zachUtil.mpdBigSigIdx], maxVals[zachUtil.mpdBigSigIdx],
                            100, minVals[zachUtil.mpdBigSigIdx], maxVals[zachUtil.mpdBigSigIdx])

sm_err_scatter = ROOT.TH2S("sm_err_scatter",
                           "sm diode mpd error change x=mpd1 y=mpd2",
                           100, minVals[zachUtil.mpdSmallSigIdx], maxVals[zachUtil.mpdSmallSigIdx],
                           100, minVals[zachUtil.mpdSmallSigIdx], maxVals[zachUtil.mpdSmallSigIdx])


lrg_prof = ROOT.TProfile("lrg_prof",
                         "lrg diode mpd change x=mpd1 y=mpd2",
                         100, 0, maxVals[zachUtil.mpdBigValIdx])

sm_prof = ROOT.TProfile("sm_prof",
                        "sm diode mpd change x=mpd1 y=mpd2",
                        100, 0, maxVals[zachUtil.mpdSmallValIdx])

lrg_err_prof = ROOT.TProfile("lrg_err_prof",
                             "lrg diode mpd error change x=mpd1 y=mpd2",
                             100, 0, maxVals[zachUtil.mpdBigSigIdx])

sm_err_prof = ROOT.TProfile("sm_err_prof",
                            "sm diode mpd error change x=mpd1 y=mpd2",
                            100, 0, maxVals[zachUtil.mpdSmallSigIdx])

# fill histograms
for twr in mpdTwrs1:
    lrgDiff    = Numeric.ravel(mpdDiff[twr, ..., zachUtil.mpdBigValIdx])
    smDiff     = Numeric.ravel(mpdDiff[twr, ..., zachUtil.mpdSmallValIdx])
    lrgErrDiff = Numeric.ravel(mpdDiff[twr, ..., zachUtil.mpdBigSigIdx])
    smErrDiff  = Numeric.ravel(mpdDiff[twr, ..., zachUtil.mpdSmallSigIdx])

    lrg1    = Numeric.ravel(mpd1[twr, ..., zachUtil.mpdBigValIdx])
    sm1     = Numeric.ravel(mpd1[twr, ..., zachUtil.mpdSmallValIdx])
    lrgErr1 = Numeric.ravel(mpd1[twr, ..., zachUtil.mpdBigSigIdx])
    smErr1  = Numeric.ravel(mpd1[twr, ..., zachUtil.mpdSmallSigIdx])

    lrg2    = Numeric.ravel(mpd2[twr, ..., zachUtil.mpdBigValIdx])
    sm2     = Numeric.ravel(mpd2[twr, ..., zachUtil.mpdSmallValIdx])
    lrgErr2 = Numeric.ravel(mpd2[twr, ..., zachUtil.mpdBigSigIdx])
    smErr2  = Numeric.ravel(mpd2[twr, ..., zachUtil.mpdSmallSigIdx])


    lrg_diff_summary.FillN(len(lrgDiff),
                           array.array('d',lrgDiff),
                           array.array('d',[1]*len(lrgDiff)))
    lrg_scatter.FillN(len(lrg1),
                      array.array('d',lrg1),
                      array.array('d',lrg2),
                      array.array('d',[1]*len(lrg1)))
    lrg_prof.FillN(len(lrg1),
                           array.array('d',lrg1),
                           array.array('d',lrg2),
                           array.array('d',[1]*len(lrg1)))


    lrg_err_diff_summary.FillN(len(lrgErrDiff),
                               array.array('d',lrgErrDiff),
                               array.array('d',[1]*len(lrgErrDiff)))
    lrg_err_scatter.FillN(len(lrgErr1),
                          array.array('d',lrgErr1),
                          array.array('d',lrgErr2),
                          array.array('d',[1]*len(lrgErr1)))
    lrg_err_prof.FillN(len(lrgErr1),
                          array.array('d',lrgErr1),
                          array.array('d',lrgErr2),
                          array.array('d',[1]*len(lrgErr1)))


    sm_diff_summary.FillN(len(smDiff),
                          array.array('d',smDiff),
                          array.array('d',[1]*len(smDiff)))
    sm_scatter.FillN(len(sm1),
                      array.array('d',sm1),
                      array.array('d',sm2),
                      array.array('d',[1]*len(sm1)))
    sm_prof.FillN(len(sm1),
                      array.array('d',sm1),
                      array.array('d',sm2),
                      array.array('d',[1]*len(sm1)))


    sm_err_diff_summary.FillN(len(smErrDiff),
                              array.array('d',smErrDiff),
                              array.array('d',[1]*len(smErrDiff)))
    sm_err_scatter.FillN(len(smErr1),
                         array.array('d',smErr1),
                         array.array('d',smErr2),
                         array.array('d',[1]*len(smErr1)))
    sm_err_prof.FillN(len(smErr1),
                         array.array('d',smErr1),
                         array.array('d',smErr2),
                         array.array('d',[1]*len(smErr1)))

lrg_prof.Fit("pol1","Q")
sm_prof.Fit("pol1","Q")
lrg_err_prof.Fit("pol1","Q")
sm_err_prof.Fit("pol1","Q")

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
