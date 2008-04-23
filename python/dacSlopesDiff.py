"""
Diff 2 CAL dacSlopes  offline calibration XML files (generate useful ROOTo plots).  The command line is:

python dacSlopesDiff.py <dacSlopes_xml_file1> <dacSlopes_xml_file2> <output_basename>

where:
    <dacSlopes_xml_file1>    = GLAST Cal dacSlopes offline calib file
    <dacSlopes_xml_file2>    = GLAST Cal dacSlopes offline calib file
    <output_basename>        =  base filename for all output files


"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL dacSlopes XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/04/21 20:25:18 $"
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
log = logging.getLogger('dacSlopesDiff')
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
dacSlopesPath1 = args[0]
dacSlopesPath2 = args[1]
outputBasename = args[2]

# read in dac xml files
log.info("Opening %s"%dacSlopesPath1)
dacSlopesFile1 = calCalibXML.calDacSlopesCalibXML(dacSlopesPath1)
log.info("Opening %s"%dacSlopesPath2)
dacSlopesFile2 = calCalibXML.calDacSlopesCalibXML(dacSlopesPath2)

# check that towers are the same (ok if they are, just print warning)
dacSlopesTwrs1 = dacSlopesFile1.getTowers()
dacSlopesTwrs2 = dacSlopesFile2.getTowers()

if (dacSlopesTwrs1 != dacSlopesTwrs2):
    log.warning("input files have different n towers")

twrSet = set(dacSlopesTwrs1) & set(dacSlopesTwrs2)
del dacSlopesTwrs1, dacSlopesTwrs2
log.info("Processing tower modules: %s"%twrSet)

# load up arrays
log.info("Reading %s"%dacSlopesPath1)
dacSlopes1 = dacSlopesFile1.read()
log.info("Reading %s"%dacSlopesPath2)
dacSlopes2 = dacSlopesFile2.read()

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
outputRootPath = outputBasename + ".root"
log.info("Opening %s"%outputRootPath)
rootFile = ROOT.TFile(outputRootPath,
                      "recreate")



# INITIALIZE HISTOGRAMS #
# extract needed arrays
(dacData1, uldData1, rngData1) = dacSlopes1
(dacData2, uldData2, rngData2) = dacSlopes2

# dac file 1
lac_slopes1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_SLOPE]
fle_slopes1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_SLOPE]
fhe_slopes1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_SLOPE]

lac_offsets1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_OFFSET]
fle_offsets1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_OFFSET]
fhe_offsets1 = dacData1[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_OFFSET]

uld_slopes1  = uldData1[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SLOPE]
uld_offsets1 = uldData1[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_OFFSET]
uld_sat1     = uldData1[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SAT]

lac_rng1 = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_LACDAC]
fle_rng1 = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FLEDAC]
fhe_rng1 = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FHEDAC]

uld_rng1 = numarray.zeros((3,16,8,2,12),numarray.Int16)
uld_rng1[0,...] = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX8]
uld_rng1[1,...] = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX1]
uld_rng1[2,...] = rngData1[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_HEX8]

# dac file 2
lac_slopes2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_SLOPE]
fle_slopes2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_SLOPE]
fhe_slopes2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_SLOPE]

lac_offsets2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_OFFSET]
fle_offsets2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_OFFSET]
fhe_offsets2 = dacData2[...,calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_OFFSET]

uld_slopes2  = uldData2[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SLOPE]
uld_offsets2 = uldData2[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_OFFSET]
uld_sat2     = uldData2[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SAT]

lac_rng2 = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_LACDAC]
fle_rng2 = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FLEDAC]
fhe_rng2 = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FHEDAC]

uld_rng2 = numarray.zeros((3,16,8,2,12),numarray.Int16)
uld_rng2[0,...] = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX8]
uld_rng2[1,...] = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX1]
uld_rng2[2,...] = rngData2[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_HEX8]

# initialize scatter plots
import os
dacSlopesFilename1 = os.path.basename(dacSlopesPath1)
dacSlopesFilename2 = os.path.basename(dacSlopesPath2)

# dac slopes
lac_slope_hist = ROOT.TH2S("lacdac_slope_scatter",
                           "lacdac_slope_scatter",
                           100,0,1,
                           100,0,1)
lac_slope_hist.GetXaxis().SetTitle(dacSlopesFilename1)
lac_slope_hist.GetYaxis().SetTitle(dacSlopesFilename2)

fle_slope_hist = ROOT.TH2S("fledac_slope_scatter",
                           "fledac_slope_scatter",
                           100,0,10,
                           100,0,10)
fle_slope_hist.GetXaxis().SetTitle(dacSlopesFilename1)
fle_slope_hist.GetYaxis().SetTitle(dacSlopesFilename2)

fhe_slope_hist = ROOT.TH2S("fhedac_slope_scatter",
                           "fhedac_slope_scatter",
                           100,0,100,
                           100,0,100)
fhe_slope_hist.GetXaxis().SetTitle(dacSlopesFilename1)
fhe_slope_hist.GetYaxis().SetTitle(dacSlopesFilename2)

uld_slope_hist = ROOT.TH2S("ulddac_slope_scatter",
                           "ulddac_slope_scatter",
                           100,0,100,
                           100,0,100)
uld_slope_hist.GetXaxis().SetTitle(dacSlopesFilename1)
uld_slope_hist.GetYaxis().SetTitle(dacSlopesFilename2)



# dac offsets
lac_offset_hist = ROOT.TH2S("lacdac_offset_scatter",
                            "lacdac_offset_scatter",
                            100,-100,100,
                            100,-100,100)
lac_offset_hist.GetXaxis().SetTitle(dacSlopesFilename1)
lac_offset_hist.GetYaxis().SetTitle(dacSlopesFilename2)

fle_offset_hist = ROOT.TH2S("fledac_offset_scatter",
                           "fledac_offset_scatter",
                            100,-100,100,
                            100,-100,100)
fle_offset_hist.GetXaxis().SetTitle(dacSlopesFilename1)
fle_offset_hist.GetYaxis().SetTitle(dacSlopesFilename2)

fhe_offset_hist = ROOT.TH2S("fhedac_offset_scatter",
                            "fhedac_offset_scatter",
                            100,-1000,1000,
                            100,-1000,1000)
fhe_offset_hist.GetXaxis().SetTitle(dacSlopesFilename1)
fhe_offset_hist.GetYaxis().SetTitle(dacSlopesFilename2)

uld_offset_hist = ROOT.TH2S("ulddac_offset_scatter",
                            "ulddac_offset_scatter",
                            100,-100,100,
                            100,-100,100)
uld_offset_hist.GetXaxis().SetTitle(dacSlopesFilename1)
uld_offset_hist.GetYaxis().SetTitle(dacSlopesFilename2)

# other 2d hists
uld_sat_hist = ROOT.TH2S("ulddac_saturation_scatter",
                         "ulddac_saturation_scatter",
                         500, 0, 10000,
                         500, 0, 10000)
uld_sat_hist.GetXaxis().SetTitle(dacSlopesFilename1)
uld_sat_hist.GetYaxis().SetTitle(dacSlopesFilename2)


# initialize DAC plots (evaluated linear model @ common threshold points
lacdac_2mev = ROOT.TH2S("lacdac_2mev",
                        "lacdac_2mev",
                        128,0,128,
                        128,0,128)
lacdac_2mev.GetXaxis().SetTitle(dacSlopesFilename1)
lacdac_2mev.GetYaxis().SetTitle(dacSlopesFilename2)

lacdac_4mev = ROOT.TH2S("lacdac_4mev",
                        "lacdac_4mev",
                        128,0,128,
                        128,0,128)
lacdac_4mev.GetXaxis().SetTitle(dacSlopesFilename1)
lacdac_4mev.GetYaxis().SetTitle(dacSlopesFilename2)

fledac_100mev = ROOT.TH2S("fledac_100mev",
                        "fledac_100mev",
                        128,0,128,
                        128,0,128)
fledac_100mev.GetXaxis().SetTitle(dacSlopesFilename1)
fledac_100mev.GetYaxis().SetTitle(dacSlopesFilename2)

fledac_150mev = ROOT.TH2S("fledac_150mev",
                        "fledac_150mev",
                        128,0,128,
                        128,0,128)
fledac_150mev.GetXaxis().SetTitle(dacSlopesFilename1)
fledac_150mev.GetYaxis().SetTitle(dacSlopesFilename2)

fhedac_1000mev = ROOT.TH2S("fhedac_1000mev",
                        "fhedac_1000mev",
                        128,0,128,
                        128,0,128)
fhedac_1000mev.GetXaxis().SetTitle(dacSlopesFilename1)
fhedac_1000mev.GetYaxis().SetTitle(dacSlopesFilename2)

fhedac_1500mev = ROOT.TH2S("fhedac_1500mev",
                        "fhedac_1500mev",
                        128,0,128,
                        128,0,128)
fhedac_1500mev.GetXaxis().SetTitle(dacSlopesFilename1)
fhedac_1500mev.GetYaxis().SetTitle(dacSlopesFilename2)

ulddac_5pct = ROOT.TH2S("ulddac_5pct",
                        "ulddac_5pct",
                        128,0,128,
                        128,0,128)
ulddac_5pct.GetXaxis().SetTitle(dacSlopesFilename1)
ulddac_5pct.GetYaxis().SetTitle(dacSlopesFilename2)

ulddac_10pct = ROOT.TH2S("ulddac_10pct",
                         "ulddac_10pct",
                         128,0,128,
                         128,0,128)
ulddac_10pct.GetXaxis().SetTitle(dacSlopesFilename1)
ulddac_10pct.GetYaxis().SetTitle(dacSlopesFilename2)

# FILL HISTOGRAMS
import array
for twr in twrSet:
    lacslope1 = numarray.ravel(lac_slopes1[twr,...])
    lacslope2 = numarray.ravel(lac_slopes2[twr,...])
    fleslope1 = numarray.ravel(fle_slopes1[twr,...])
    fleslope2 = numarray.ravel(fle_slopes2[twr,...])
    fheslope1 = numarray.ravel(fhe_slopes1[twr,...])
    fheslope2 = numarray.ravel(fhe_slopes2[twr,...])
    uldslope1 = numarray.ravel(uld_slopes1[:,twr,...])
    uldslope2 = numarray.ravel(uld_slopes2[:,twr,...])

    lacoffset1 = numarray.ravel(lac_offsets1[twr,...])
    lacoffset2 = numarray.ravel(lac_offsets2[twr,...])
    fleoffset1 = numarray.ravel(fle_offsets1[twr,...])
    fleoffset2 = numarray.ravel(fle_offsets2[twr,...])
    fheoffset1 = numarray.ravel(fhe_offsets1[twr,...])
    fheoffset2 = numarray.ravel(fhe_offsets2[twr,...])
    uldoffset1 = numarray.ravel(uld_offsets1[:,twr,...])
    uldoffset2 = numarray.ravel(uld_offsets2[:,twr,...])

    uldsat1 = numarray.ravel(uld_sat1[:,twr,...])
    uldsat2 = numarray.ravel(uld_sat2[:,twr,...])

    lacrng1 = numarray.ravel(lac_rng1[twr,...])
    lacrng2 = numarray.ravel(lac_rng2[twr,...])
    flerng1 = numarray.ravel(fle_rng1[twr,...])
    flerng2 = numarray.ravel(fle_rng2[twr,...])
    fherng1 = numarray.ravel(fhe_rng1[twr,...])
    fherng2 = numarray.ravel(fhe_rng2[twr,...])
    uldrng1 = numarray.ravel(uld_rng1[:,twr,...])
    uldrng2 = numarray.ravel(uld_rng2[:,twr,...])

    # calculate dac settings for known thresholds
    lac2mev1    = (2    - lacoffset1)/lacslope1 + lacrng1*64
    lac4mev1    = (4    - lacoffset1)/lacslope1 + lacrng1*64
    fle100mev1  = (100  - fleoffset1)/fleslope1 + flerng1*64
    fle150mev1  = (150  - fleoffset1)/fleslope1 + flerng1*64
    fhe1000mev1 = (1000 - fheoffset1)/fheslope1 + fherng1*64
    fhe1500mev1 = (1500 - fheoffset1)/fheslope1 + fherng1*64
    uld5pct1    = (uldsat1*.95 - uldoffset1)/uldslope1 + uldrng1*64
    uld10pct1    = (uldsat1*.90 - uldoffset1)/uldslope1 + uldrng1*64

    lac2mev2    = (2    - lacoffset2)/lacslope2 + lacrng2*64
    lac4mev2    = (4    - lacoffset2)/lacslope2 + lacrng2*64
    fle100mev2  = (100  - fleoffset2)/fleslope2 + flerng2*64
    fle150mev2  = (150  - fleoffset2)/fleslope2 + flerng2*64
    fhe1000mev2 = (1000 - fheoffset2)/fheslope2 + fherng2*64
    fhe1500mev2 = (1500 - fheoffset2)/fheslope2 + fherng2*64
    uld5pct2    = (uldsat2*.95 - uldoffset2)/uldslope2 + uldrng2*64
    uld10pct2    = (uldsat2*.90 - uldoffset2)/uldslope2 + uldrng2*64

    # 'weight' arrays all equal 1
    dac_weights = array.array('d',[1]*len(lacslope1))
    ulddac_weights = array.array('d',[1]*len(uldslope1))

    lac_slope_hist.FillN(len(lacslope1),
                         array.array('d',lacslope1),
                         array.array('d',lacslope2),
                         dac_weights)
    fle_slope_hist.FillN(len(fleslope1),
                         array.array('d',fleslope1),
                         array.array('d',fleslope2),
                         dac_weights)
    fhe_slope_hist.FillN(len(fheslope1),
                         array.array('d',fheslope1),
                         array.array('d',fheslope2),
                         dac_weights)
    uld_slope_hist.FillN(len(uldslope1),
                         array.array('d',uldslope1),
                         array.array('d',uldslope2),
                         ulddac_weights)


    lac_offset_hist.FillN(len(lacoffset1),
                         array.array('d',lacoffset1),
                         array.array('d',lacoffset2),
                          dac_weights)
    fle_offset_hist.FillN(len(fleoffset1),
                         array.array('d',fleoffset1),
                         array.array('d',fleoffset2),
                          dac_weights)
    fhe_offset_hist.FillN(len(fheoffset1),
                         array.array('d',fheoffset1),
                         array.array('d',fheoffset2),
                          dac_weights)
    uld_offset_hist.FillN(len(uldoffset1),
                         array.array('d',uldoffset1),
                         array.array('d',uldoffset2),
                         ulddac_weights)

    uld_sat_hist.FillN(len(uldsat1),
                       array.array('d',uldsat1),
                       array.array('d',uldsat2),
                       ulddac_weights)


    lacdac_2mev.FillN(len(lac2mev1),
                      array.array('d',lac2mev1),
                      array.array('d',lac2mev2),
                      dac_weights)
    lacdac_4mev.FillN(len(lac4mev1),
                      array.array('d',lac4mev1),
                      array.array('d',lac4mev2),
                      dac_weights)
    fledac_100mev.FillN(len(fle100mev1),
                      array.array('d',fle100mev1),
                      array.array('d',fle100mev2),
                      dac_weights)
    fledac_150mev.FillN(len(fle150mev1),
                        array.array('d',fle150mev1),
                        array.array('d',fle150mev2),
                        dac_weights)
    fhedac_1000mev.FillN(len(fhe1000mev1),
                         array.array('d',fhe1000mev1),
                         array.array('d',fhe1000mev2),
                         dac_weights)
    fhedac_1500mev.FillN(len(fhe1500mev1),
                         array.array('d',fhe1500mev1),
                         array.array('d',fhe1500mev2),
                         dac_weights)

    ulddac_5pct.FillN(len(uld5pct1),
                      array.array('d',uld5pct1),
                      array.array('d',uld5pct2),
                      ulddac_weights)
    ulddac_10pct.FillN(len(uld10pct1),
                      array.array('d',uld10pct1),
                      array.array('d',uld10pct2),
                      ulddac_weights)

# GENERATE REPORT #
rptFilename = outputBasename + ".ps"
# setup plotting options
ROOT.gStyle.SetPalette(1)

# print first page in doc
lacdac_2mev.Draw("colZ")
ROOT.gPad.SetGrid()
ROOT.gPad.Print(rptFilename+"(")

# print middle pages
lacdac_4mev.Draw("colZ")
ROOT.gPad.Print(rptFilename)

fledac_100mev.Draw("colZ")
ROOT.gPad.Print(rptFilename)

fledac_150mev.Draw("colZ")
ROOT.gPad.Print(rptFilename)

fhedac_1000mev.Draw("colZ")
ROOT.gPad.Print(rptFilename)

fhedac_1500mev.Draw("colZ")
ROOT.gPad.Print(rptFilename)

ulddac_5pct.Draw("colZ")
ROOT.gPad.Print(rptFilename)

# print last page in doc
ulddac_10pct.Draw("colZ")
ROOT.gPad.Print(rptFilename+")")


log.info("Writing %s"%outputRootPath)
rootFile.Write()
rootFile.Close()
