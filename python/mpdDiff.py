"""
Diff 2 CAL mevPerDAC (mevPerDAC) offline calibration XML files.  The command line is:

python mpdDiff.py <mpd_xml_file1> <mpd_xml_file2> <output_basename>

where:
    <mpd_xml_file1>    = GLAST Cal mevPerDAC offline calib file
    <mpd_xml_file2>    = GLAST Cal mevPerDAC offline calib file
    <output_basename>  = base filename for all output files


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL mevPerDAC XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/04/23 22:35:14 $"
__version__     = "$Revision: 1.9 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import logging
import array
import numarray


import calCalibXML
import calConstant
import cgc_util


# setup logger

logging.basicConfig()
log = logging.getLogger('mpdDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(__doc__)
    sys.exit(1)


# get filenames
mpdPath1 = sys.argv[1]
mpdPath2 = sys.argv[2]
outputBasename = sys.argv[3]

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
rootPath = outputBasename + ".root"
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "mpdDiff(%s,%s)"%(mpdPath1,mpdPath2))

# calc diffs
mpdDiff = (mpd2 - mpd1)/mpd1

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
    for idx in range(cgc_util.N_MPD_IDX):
        # first pass only compare against 1st set
        if minVals is None:
            minVals = numarray.ones(cgc_util.N_MPD_IDX,'f')*1e5
            maxVals = numarray.ones(cgc_util.N_MPD_IDX,'f')*-1e5
            minDiff = numarray.ones(cgc_util.N_MPD_IDX,'f')*1e5
            maxDiff = numarray.ones(cgc_util.N_MPD_IDX,'f')*-1e5

            minVals[idx] = min(numarray.ravel(mpd1[twr, ..., idx]))
            maxVals[idx] = max(numarray.ravel(mpd1[twr, ..., idx]))
            minVals[idx] = min(minVals[idx], min(numarray.ravel(mpd2[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(numarray.ravel(mpd2[twr, ..., idx])))

            minDiff[idx] = min(numarray.ravel(mpdDiff[twr, ..., idx]))
            maxDiff[idx] = max(numarray.ravel(mpdDiff[twr, ..., idx]))

        else:
            minVals[idx] = min(minVals[idx], min(numarray.ravel(mpd1[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(numarray.ravel(mpd1[twr, ..., idx])))
            minVals[idx] = min(minVals[idx], min(numarray.ravel(mpd2[twr, ..., idx])))
            maxVals[idx] = max(maxVals[idx], max(numarray.ravel(mpd2[twr, ..., idx])))

            minDiff[idx] = min(minDiff[idx], min(numarray.ravel(mpdDiff[twr, ..., idx])))
            maxDiff[idx] = max(maxDiff[idx], max(numarray.ravel(mpdDiff[twr, ..., idx])))


# gobal summary histograms
import os
mpdFilename1 = os.path.basename(mpdPath1)
mpdFilename2 = os.path.basename(mpdPath2)

lrg_reldiff_summary = ROOT.TH1I("lrg_reldiff_summary",
                                "lrg_reldiff_summary (mpd2 - mpd1)",
                                50, minDiff[cgc_util.mpdBigValIdx], maxDiff[cgc_util.mpdBigValIdx])
lrg_reldiff_summary.GetXaxis().SetTitle(mpdFilename1)
lrg_reldiff_summary.GetYaxis().SetTitle(mpdFilename2)


lrg_err_reldiff_summary = ROOT.TH1I("lrg_err_reldiff_summary",
                                    "lrg_err_reldiff_summary (mpd2 - mpd)",
                                    50, minDiff[cgc_util.mpdBigSigIdx], maxDiff[cgc_util.mpdBigSigIdx])
lrg_err_reldiff_summary.GetXaxis().SetTitle(mpdFilename1)
lrg_err_reldiff_summary.GetYaxis().SetTitle(mpdFilename2)


sm_reldiff_summary = ROOT.TH1I("sm_reldiff_summary",
                               "sm_reldiff_summary (mpd2 - mpd1)",
                               50, minDiff[cgc_util.mpdSmallValIdx], maxDiff[cgc_util.mpdSmallValIdx])
sm_reldiff_summary.GetXaxis().SetTitle(mpdFilename1)
sm_reldiff_summary.GetYaxis().SetTitle(mpdFilename2)


sm_err_reldiff_summary = ROOT.TH1I("sm_err_reldiff_summary",
                                   "sm_err_reldiff_summary (mpd2 - mpd1)",
                                   50, minDiff[cgc_util.mpdSmallSigIdx], maxDiff[cgc_util.mpdSmallSigIdx])
sm_err_reldiff_summary.GetXaxis().SetTitle(mpdFilename1)
sm_err_reldiff_summary.GetYaxis().SetTitle(mpdFilename2)



# build scatter histograms

lrg_scatter = ROOT.TH2S("lrg_scatter",
                        "lrg diode mpd change x=mpd1 y=mpd2",
                        100, minVals[cgc_util.mpdBigValIdx], maxVals[cgc_util.mpdBigValIdx],
                        100, minVals[cgc_util.mpdBigValIdx], maxVals[cgc_util.mpdBigValIdx])
lrg_scatter.GetXaxis().SetTitle(mpdFilename1)
lrg_scatter.GetYaxis().SetTitle(mpdFilename2)

sm_scatter = ROOT.TH2S("sm_scatter",
                       "sm diode mpd change x=mpd1 y=mpd2",
                       100, minVals[cgc_util.mpdSmallValIdx], maxVals[cgc_util.mpdSmallValIdx],
                       100, minVals[cgc_util.mpdSmallValIdx], maxVals[cgc_util.mpdSmallValIdx])
sm_scatter.GetXaxis().SetTitle(mpdFilename1)
sm_scatter.GetYaxis().SetTitle(mpdFilename2)

lrg_err_scatter = ROOT.TH2S("lrg_err_scatter",
                            "lrg diode mpd error change x=mpd1 y=mpd2",
                            100, minVals[cgc_util.mpdBigSigIdx], maxVals[cgc_util.mpdBigSigIdx],
                            100, minVals[cgc_util.mpdBigSigIdx], maxVals[cgc_util.mpdBigSigIdx])
lrg_err_scatter.GetXaxis().SetTitle(mpdFilename1)
lrg_err_scatter.GetYaxis().SetTitle(mpdFilename2)

sm_err_scatter = ROOT.TH2S("sm_err_scatter",
                           "sm diode mpd error change x=mpd1 y=mpd2",
                           100, minVals[cgc_util.mpdSmallSigIdx], maxVals[cgc_util.mpdSmallSigIdx],
                           100, minVals[cgc_util.mpdSmallSigIdx], maxVals[cgc_util.mpdSmallSigIdx])
sm_err_scatter.GetXaxis().SetTitle(mpdFilename1)
sm_err_scatter.GetYaxis().SetTitle(mpdFilename2)


lrg_prof = ROOT.TProfile("lrg_prof",
                         "lrg diode mpd change x=mpd1 y=mpd2",
                         100, 0, maxVals[cgc_util.mpdBigValIdx])
lrg_prof.GetXaxis().SetTitle(mpdFilename1)
lrg_prof.GetYaxis().SetTitle(mpdFilename2)

sm_prof = ROOT.TProfile("sm_prof",
                        "sm diode mpd change x=mpd1 y=mpd2",
                        100, 0, maxVals[cgc_util.mpdSmallValIdx])
sm_prof.GetXaxis().SetTitle(mpdFilename1)
sm_prof.GetYaxis().SetTitle(mpdFilename2)

lrg_err_prof = ROOT.TProfile("lrg_err_prof",
                             "lrg diode mpd error change x=mpd1 y=mpd2",
                             100, 0, maxVals[cgc_util.mpdBigSigIdx])
lrg_err_prof.GetXaxis().SetTitle(mpdFilename1)
lrg_err_prof.GetYaxis().SetTitle(mpdFilename2)

sm_err_prof = ROOT.TProfile("sm_err_prof",
                            "sm diode mpd error change x=mpd1 y=mpd2",
                            100, 0, maxVals[cgc_util.mpdSmallSigIdx])
sm_err_prof.GetXaxis().SetTitle(mpdFilename1)
sm_err_prof.GetYaxis().SetTitle(mpdFilename2)

# fill histograms
for twr in mpdTwrs1:
    lrgDiff    = numarray.ravel(mpdDiff[twr, ..., cgc_util.mpdBigValIdx])
    smDiff     = numarray.ravel(mpdDiff[twr, ..., cgc_util.mpdSmallValIdx])
    lrgErrDiff = numarray.ravel(mpdDiff[twr, ..., cgc_util.mpdBigSigIdx])
    smErrDiff  = numarray.ravel(mpdDiff[twr, ..., cgc_util.mpdSmallSigIdx])

    lrg1    = numarray.ravel(mpd1[twr, ..., cgc_util.mpdBigValIdx])
    sm1     = numarray.ravel(mpd1[twr, ..., cgc_util.mpdSmallValIdx])
    lrgErr1 = numarray.ravel(mpd1[twr, ..., cgc_util.mpdBigSigIdx])
    smErr1  = numarray.ravel(mpd1[twr, ..., cgc_util.mpdSmallSigIdx])

    lrg2    = numarray.ravel(mpd2[twr, ..., cgc_util.mpdBigValIdx])
    sm2     = numarray.ravel(mpd2[twr, ..., cgc_util.mpdSmallValIdx])
    lrgErr2 = numarray.ravel(mpd2[twr, ..., cgc_util.mpdBigSigIdx])
    smErr2  = numarray.ravel(mpd2[twr, ..., cgc_util.mpdSmallSigIdx])


    lrg_reldiff_summary.FillN(len(lrgDiff),
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


    lrg_err_reldiff_summary.FillN(len(lrgErrDiff),
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


    sm_reldiff_summary.FillN(len(smDiff),
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


    sm_err_reldiff_summary.FillN(len(smErrDiff),
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

# GENERATE POSTSCRIPT REPORT #
rptFilename = outputBasename + ".ps"
# setup plotting options
ROOT.gStyle.SetPalette(1)

# print first page in doc
lrg_scatter.Draw("colZ")
ROOT.gPad.SetGrid()
ROOT.gPad.Print(rptFilename+"(")

# print middle pages
sm_scatter.Draw("colZ")
ROOT.gPad.Print(rptFilename)

lrg_err_scatter.Draw("colZ")
ROOT.gPad.Print(rptFilename)

# print last page in doc
sm_err_scatter.Draw("colZ")
ROOT.gPad.Print(rptFilename+")")


log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
