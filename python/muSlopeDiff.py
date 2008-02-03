"""
Diff 2 CAL muSlope (muSlope) offline calibration XML files.  The command line is:

python muSlopeDiff.py [-p] [-x xaxis_width] <muSlope_xml_file1> <muSlope_xml_file2> <output_root_file>

where:
    -p                  = pctdiff (default = absolute diff)
    -x xaxis_width      = fixed x axis scaling (default = auto_scale
    <muSlope_xml_file1> = GLAST Cal muSlope offline calib file
    <muSlope_xml_file2> = GLAST Cal muSlope offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL muSlope XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/08/17 16:35:28 $"
__version__     = "$Revision: 1.2 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import logging
import array
import Numeric
import getopt


import calCalibXML
import calConstant
import cgc_util

# CONSTANTS #
OPTYPE_DIFF    = 0
OPTYPE_PCTDIFF = 1


# setup logger

logging.basicConfig()
log = logging.getLogger('muSlopeDiff')
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
muSlopePath1 = args[0]
muSlopePath2 = args[1]
rootPath = args[2]

# read in dac xml files
log.info("Opening %s"%muSlopePath1)
muSlopeFile1 = calCalibXML.calMuSlopeCalibXML(muSlopePath1)
log.info("Opening %s"%muSlopePath2)
muSlopeFile2 = calCalibXML.calMuSlopeCalibXML(muSlopePath2)

# check that towers are the same (ok if they are, just print warning)
muSlopeTwrs1 = muSlopeFile1.getTowers()
muSlopeTwrs2 = muSlopeFile2.getTowers()

if (muSlopeTwrs1 != muSlopeTwrs2):
    log.error("input files have different n towers.  I quit! ;)")


# load up arrays
log.info("Reading %s"%muSlopePath1)
muSlope1 = muSlopeFile1.read()
log.info("Reading %s"%muSlopePath2)
muSlope2 = muSlopeFile2.read()

# calc diffs
if optype == OPTYPE_DIFF:
    diff = muSlope2 - muSlope1
elif optype == OPTYPE_PCTDIFF:
    diff = (muSlope2 - muSlope1)*100/muSlope1
else:
    log.error("Undefined optype, programmer error...")
    sys.exit(-1)

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "muSlope%s(%s,%s)"%(optypeName,muSlopePath1,muSlopePath2))

# gobal summary histograms
muSlopeHists = {}
sigHists = {}
for rng in range(calConstant.NUM_RNG):
    muSlopeHists[rng] = ROOT.TH1S("EnergyPerBin%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              "EnergyPerBin%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              nbins,xaxismin,xaxismax)
    
    sigHists[rng] = ROOT.TH1S("EnergyPerBinSigma%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              "EnergyPerBinSigma%s_%s"%(optypeName,calConstant.CRNG[rng]),
                              nbins,xaxismin,xaxismax)

for twr in muSlopeTwrs1:
    # from calCalibXML.py
    #         Param: muSlopeData -
    #             A Numeric array containing the muSlope data
    #             of shape (16, 8, 2, 12, 4, 3) The last dimension contains
    #             the following data for each crystal end and energy
    #             range:
    #                 0 = avg value
    #                 1 = sig value
    #                 2 = cos values

    for rng in range(calConstant.NUM_RNG):
        muSlopeDiff = diff[twr,...,rng,0]
        sigDiff = diff[twr,...,rng,1]
        
        for p in Numeric.ravel(muSlopeDiff):
            muSlopeHists[rng].Fill(p)
        for s in Numeric.ravel(sigDiff):
            sigHists[rng].Fill(s)

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
