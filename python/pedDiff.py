"""
Diff 2 CAL pedestal (pedestal) offline calibration XML files.  The command line is:

python pedDiff.py [-p] [-x xaxis_width] <ped_xml_file1> <ped_xml_file2> <output_root_file>

where:
    -p                 = pctdiff (default = absolute diff)
    -x xaxis_width     = fixed x axis scaling (default = auto_scale
    <ped_xml_file1>    = GLAST Cal pedestal offline calib file
    <ped_xml_file2>    = GLAST Cal pedestal offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL pedestal XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/11 21:35:58 $"
__version__     = "$Revision: 1.5 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import logging
import array
import numarray
import getopt


import calCalibXML
import calConstant
import cgc_util

# CONSTANTS #
OPTYPE_DIFF    = 0
OPTYPE_PCTDIFF = 1


# setup logger

logging.basicConfig()
log = logging.getLogger('pedDiff')
log.setLevel(logging.INFO)

# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "px:")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

optype      = OPTYPE_DIFF
optypeName  = "Diff"
xaxismin    = 0
xaxismax    = 0
nbins       = 100
for o, a in opts:
    if o == "-p":
        optype      = OPTYPE_PCTDIFF
        optypeName = "PctDiff"
    elif o == "-x":
        xaxismax = float(a)
        xaxismin = -1*xaxismax
    else:
        log.error("Invalid option: %s. %s"%(l,__doc__))
        sys.exit(-1)

# now check for req'd params
if len(args) != 3:
    log.error("bad n args: " + str(len(args)) + " " + __doc__)
    sys.exit(1)

# get filenames
pedPath1 = args[0]
pedPath2 = args[1]
rootPath = args[2]

# read in dac xml files
log.info("Opening %s"%pedPath1)
pedFile1 = calCalibXML.calPedCalibXML(pedPath1)
log.info("Opening %s"%pedPath2)
pedFile2 = calCalibXML.calPedCalibXML(pedPath2)

# check that towers are the same (ok if they are, just print warning)
pedTwrs1 = pedFile1.getTowers()
pedTwrs2 = pedFile2.getTowers()

if (pedTwrs1 != pedTwrs2):
    log.warning("input files have different n towers.  I quit! ;)")

twrSet = pedTwrs1 & pedTwrs2
del pedTwrs1, pedTwrs2 # don't want to get confused & use these variables any more (twrSet subsumes them)
log.info("Processing tower modules: %s"%twrSet)

# load up arrays
log.info("Reading %s"%pedPath1)
ped1 = pedFile1.read()
log.info("Reading %s"%pedPath2)
ped2 = pedFile2.read()

# calc diffs
if optype == OPTYPE_DIFF:
    diff = ped2 - ped1
elif optype == OPTYPE_PCTDIFF:
    diff = (ped2 - ped1)*100/ped1
else:
    log.error("Undefined optype, programmer error...")
    sys.exit(-1)

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "ped%s(%s,%s)"%(optypeName,pedPath1,pedPath2))

# gobal summary histograms
pedHists = {}
sigHists = {}
for rng in range(calConstant.NUM_RNG):
    pedHists[rng] = ROOT.TH1S("Pedestal%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              "Pedestal%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              nbins,xaxismin,xaxismax)
    
    sigHists[rng] = ROOT.TH1S("PedestalSigma%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              "PedestalSigma%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              nbins,xaxismin,xaxismax)

for twr in twrSet:
    # from calCalibXML.py
    #         Param: pedData -
    #             A numarray array containing the pedestal data
    #             of shape (16, 8, 2, 12, 4, 3) The last dimension contains
    #             the following data for each crystal end and energy
    #             range:
    #                 0 = avg value
    #                 1 = sig value
    #                 2 = cos values

    for rng in range(calConstant.NUM_RNG):
        pedDiff = diff[twr,...,rng,0]
        sigDiff = diff[twr,...,rng,1]
        
        for p in numarray.ravel(pedDiff):
            pedHists[rng].Fill(p)
        for s in numarray.ravel(sigDiff):
            sigHists[rng].Fill(s)

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
