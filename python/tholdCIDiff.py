"""
Diff 2 CAL tholdCI  offline calibration XML files (generate useful ROOT plots).  The command line is:

python tholdCIDiff.py <tholdCI_xml_file1> <tholdCI_xml_file2> <output_basename>

where:
    <tholdCI_xml_file1>    = GLAST Cal tholdCI offline calib file
    <tholdCI_xml_file2>    = GLAST Cal tholdCI offline calib file
    <output_basename>        =  base filename for all output files


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL tholdCI XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/04/29 15:46:36 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
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

# setup logger
logging.basicConfig()
log = logging.getLogger('tholdCIDiff')
log.setLevel(logging.INFO)

# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

for o, a in opts:
    log.error("Invalid option: %s. \n%s"%(o,__doc__))
    sys.exit(-1)

# now check for req'd params
if len(args) != 3:
    log.error("bad n args: " + str(len(args)) + " " + __doc__)
    sys.exit(1)

# get filenames
tholdCIPath1 = args[0]
tholdCIPath2 = args[1]
outputBasename = args[2]

# read in tholdCI xml files
log.info("Opening %s"%tholdCIPath1)
tholdCIFile1 = calCalibXML.calTholdCICalibXML(tholdCIPath1)
log.info("Opening %s"%tholdCIPath2)
tholdCIFile2 = calCalibXML.calTholdCICalibXML(tholdCIPath2)

# check that towers are the same (ok if they are, just print warning)
tholdCITwrs1 = tholdCIFile1.getTowers()
tholdCITwrs2 = tholdCIFile2.getTowers()

if (tholdCITwrs1 != tholdCITwrs2):
    log.warning("input files have different n towers")

twrSet = set(tholdCITwrs1) & set(tholdCITwrs2)
del tholdCITwrs1, tholdCITwrs2
log.info("Processing tower modules: %s"%twrSet)

# load up arrays
log.info("Reading %s"%tholdCIPath1)
tholdCI1 = tholdCIFile1.read()
log.info("Reading %s"%tholdCIPath2)
tholdCI2 = tholdCIFile2.read()

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
outputRootPath = outputBasename + ".root"
log.info("Opening %s"%outputRootPath)
rootFile = ROOT.TFile(outputRootPath,
                      "recreate")

# INITIALIZE HISTOGRAMS #
# extract needed arrays
(adcData1, uldData1, pedData1) = tholdCI1
(adcData2, uldData2, pedData2) = tholdCI2

# initialize scatter plots
import os
tholdCIFilename1 = os.path.basename(tholdCIPath1)
tholdCIFilename2 = os.path.basename(tholdCIPath2)

lac_scatter_hist = ROOT.TH2S("lac_thold_scatter",
                             "lac_thold_scatter",
                             200,0,200,
                             200,0,200)
lac_scatter_hist.GetXaxis().SetTitle(tholdCIFilename1)
lac_scatter_hist.GetYaxis().SetTitle(tholdCIFilename2)

lac_reldiff_hist = ROOT.TH1S("lac_relative_diff",
                             "lac_relative_diff",
                             100,-1,1)
lac_reldiff_hist.GetXaxis().SetTitle(tholdCIFilename1)
lac_reldiff_hist.GetYaxis().SetTitle(tholdCIFilename2)

fle_scatter_hist = ROOT.TH2S("fle_thold_scatter",
                             "fle_thold_scatter",
                             200,2000,6000,
                             200,2000,6000)
fle_scatter_hist.GetXaxis().SetTitle(tholdCIFilename1)
fle_scatter_hist.GetYaxis().SetTitle(tholdCIFilename2)

fle_reldiff_hist = ROOT.TH1S("fle_relative_diff",
                          "fle_relative_diff",
                          100,-1,1)
fle_reldiff_hist.GetXaxis().SetTitle(tholdCIFilename1)
fle_reldiff_hist.GetYaxis().SetTitle(tholdCIFilename2)


fhe_scatter_hist = ROOT.TH2S("fhe_thold_scatter",
                             "fhe_thold_scatter",
                             200,200,800,
                             200,200,800)
fhe_scatter_hist.GetXaxis().SetTitle(tholdCIFilename1)
fhe_scatter_hist.GetYaxis().SetTitle(tholdCIFilename2)

fhe_reldiff_hist = ROOT.TH1S("fhe_relative_diff",
                          "fhe_relative_diff",
                          100,-1,1)
fhe_reldiff_hist.GetXaxis().SetTitle(tholdCIFilename1)
fhe_reldiff_hist.GetYaxis().SetTitle(tholdCIFilename2)

# only do ULD threshold for bottom 3 ranges
uld_scatter_hist = {}
uld_reldiff_hist = {}
for rng in range(3):
    uld_scatter_hist[rng] = ROOT.TH2S("uld_thold_scatter_%s"%calConstant.CRNG[rng],
                                      "uld_thold_scatter_%s"%calConstant.CRNG[rng],
                                      200,3095,4095,
                                      200, 3095, 4095)
    uld_scatter_hist[rng].GetXaxis().SetTitle(tholdCIFilename1)
    uld_scatter_hist[rng].GetYaxis().SetTitle(tholdCIFilename2)
    
    uld_reldiff_hist[rng] = ROOT.TH1S("uld_relative_reldiff_%s"%calConstant.CRNG[rng],
                                      "uld_relative_reldiff_%s"%calConstant.CRNG[rng],
                                      100,-1,1)
    uld_reldiff_hist[rng].GetXaxis().SetTitle(tholdCIFilename1)
    uld_reldiff_hist[rng].GetYaxis().SetTitle(tholdCIFilename2)

# FILL HISTOGRAMS
import array
for twr in twrSet:
    # 'weight' arrays all equal 1

    lac1 = numarray.ravel(adcData1[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_LAC])
    lac2 = numarray.ravel(adcData2[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_LAC])
    dac_weights = array.array('d',[1]*len(lac1))
    lac_scatter_hist.FillN(len(lac1),
                           array.array('d',lac1),
                           array.array('d',lac2),
                           dac_weights)
    lac_reldiff_hist.FillN(len(lac1),
                           array.array('d',(lac2-lac1)/lac1),
                           dac_weights)


    fle1 = numarray.ravel(adcData1[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_FLE])
    fle2 = numarray.ravel(adcData2[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_FLE])
    fle_scatter_hist.FillN(len(fle1),
                           array.array('d',fle1),
                           array.array('d',fle2),
                           dac_weights)
    fle_reldiff_hist.FillN(len(fle1),
                           array.array('d',(fle2-fle1)/fle1),
                           dac_weights)


    fhe1 = numarray.ravel(adcData1[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_FHE])
    fhe2 = numarray.ravel(adcData2[twr,...,calCalibXML.calTholdCICalibXML.ADCDATA_FHE])
    fhe_scatter_hist.FillN(len(fhe1),
                           array.array('d',fhe1),
                           array.array('d',fhe2),
                           dac_weights)
    fhe_reldiff_hist.FillN(len(fhe1),
                           array.array('d',(fhe2-fhe1)/fhe1),
                           dac_weights)

    for rng in range(3):
        uld1 = numarray.ravel(uldData1[twr,...,rng])
        uld2 = numarray.ravel(uldData2[twr,...,rng])
        ulddac_weights = array.array('d',[1]*len(numarray.ravel(uld1)))
        uld_scatter_hist[rng].FillN(len(uld1),
                                    array.array('d',uld1),
                                    array.array('d',uld2),
                                    ulddac_weights)
        uld_reldiff_hist[rng].FillN(len(uld1),
                                    array.array('d',(uld2-uld1)/uld1),
                                    ulddac_weights)

        

# GENERATE PLOTS #

# setup plotting options
ROOT.gStyle.SetPalette(1)

c = ROOT.TCanvas("lac_scatter", "lac_scatter", -1)
c.SetGrid()
lac_scatter_hist.Draw("colZ")
c.Write()

c = ROOT.TCanvas("fle_scatter", "fle_scatter", -1)
c.SetGrid()
fle_scatter_hist.Draw("colZ")
c.Write()

c = ROOT.TCanvas("fhe_scatter", "fhe_scatter", -1)
c.SetGrid()
fhe_scatter_hist.Draw("colZ")
c.Write()

for rng in range(3):
    c = ROOT.TCanvas("uld_scatter_%s"%calConstant.CRNG[rng], "uld_scatter_%s"%calConstant.CRNG[rng], -1)
    c.SetGrid()
    uld_scatter_hist[rng].Draw("colZ")
    c.Write()

# close ROOT file
log.info("Writing %s"%outputRootPath)
rootFile.Write()
rootFile.Close()
