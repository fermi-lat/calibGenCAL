"""
Diff 2 CAL pedestal (pedestal) offline calibration XML files.  The command line is:

python pedDiff.py <ped_xml_file1> <ped_xml_file2> <output_root_file>

where:
    <ped_xml_file1>    = GLAST Cal pedestal offline calib file
    <ped_xml_file2>    = GLAST Cal pedestal offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL pedestal XML files."
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

usage = "Usage: python pedDiff.py <ped_xml_file1> <ped_xml_file2> <output_root_file>"

# setup logger

logging.basicConfig()
log = logging.getLogger('pedDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(usage)
    sys.exit(1)


# get filenames
pedPath1 = sys.argv[1]
pedPath2 = sys.argv[2]
rootPath = sys.argv[3]

# read in dac xml files
log.info("Opening %s"%pedPath1)
pedFile1 = calCalibXML.calPedCalibXML(pedPath1)
log.info("Opening %s"%pedPath2)
pedFile2 = calCalibXML.calPedCalibXML(pedPath2)

# check that towers are the same (ok if they are, just print warning)
pedTwrs1 = pedFile1.getTowers()
pedTwrs2 = pedFile2.getTowers()

if (pedTwrs1 != pedTwrs2):
    log.error("input files have different n towers.  I quit! ;)")


# load up arrays
log.info("Reading %s"%pedPath1)
ped1 = pedFile1.read()
log.info("Reading %s"%pedPath2)
ped2 = pedFile2.read()

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "pedDiff(%s,%s)"%(pedPath1,pedPath2))

# calc diffs
diff = ped2 - ped1

# gobal summary histograms
pedHists = {}
sigHists = {}
for rng in range(calConstant.NUM_RNG):
    pedHists[rng] = ROOT.TH1S("pedDiff_%s"%calConstant.CRNG[rng],
                              "pedDiff_%s"%calConstant.CRNG[rng],
                              100,0,0)

    sigHists[rng] = ROOT.TH1S("sigDiff_%s"%calConstant.CRNG[rng],
                              "sigDiff_%s"%calConstant.CRNG[rng],
                              100,0,0)

for twr in pedTwrs1:
    # from calCalibXML.py
    #         Param: pedData -
    #             A Numeric array containing the pedestal data
    #             of shape (16, 8, 2, 12, 4, 3) The last dimension contains
    #             the following data for each crystal end and energy
    #             range:
    #                 0 = avg value
    #                 1 = sig value
    #                 2 = cos values

    for rng in range(calConstant.NUM_RNG):
        pedDiff = diff[twr,...,rng,0]
        sigDiff = diff[twr,...,rng,1]
        
        for p in Numeric.ravel(pedDiff):
            pedHists[rng].Fill(p)
        for s in Numeric.ravel(sigDiff):
            sigHists[rng].Fill(s)

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()

















