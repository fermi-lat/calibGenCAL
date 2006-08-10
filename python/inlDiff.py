"""
Diff 2 CAL CIDAC2ADC (intNonlin) offline calibration XML files.  The command line is:

python inlDiff.py <inl_xml_file1> <inl_xml_file2> <output_root_file>

where:
    <inl_xml_file1>    = GLAST Cal cidac2adc offline calib file
    <inl_xml_file2>    = GLAST Cal cidac2adc offline calib file
    <output_root_file> = ROOT overplots & residuals will be saved here.

    
"""

__facility__    = "Offline"
__abstract__    = "Diff 2 CAL CIDAC2ADC XML files."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2006/07/13 20:25:13 $"
__version__     = "$Revision: 1.2 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calCalibXML
import calConstant
import logging
import zachUtil
import array

usage = "Usage: python inlDiff.py <inl_xml_file1> <inl_xml_file2> <output_root_file>"

# setup logger

logging.basicConfig()
log = logging.getLogger('inlDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(usage)
    sys.exit(1)


# get filenames
inlPath1 = sys.argv[1]
inlPath2 = sys.argv[2]
rootPath = sys.argv[3]

# read in dac xml files
log.info("Opening %s"%inlPath1)
inlFile1 = calCalibXML.calIntNonlinCalibXML(inlPath1)
log.info("Opening %s"%inlPath2)
inlFile2 = calCalibXML.calIntNonlinCalibXML(inlPath2)

# check that towers are the same (ok if they are, just print warning)
inlTwrs1 = inlFile1.getTowers()
inlTwrs2 = inlFile2.getTowers()

if (inlTwrs1 != inlTwrs2):
    log.error("input files have different nTowers.  I quit! ;)")


# load up arrays
log.info("Reading %s"%inlPath1)
inl1 = inlFile1.read()
log.info("Reading %s"%inlPath2)
inl2 = inlFile2.read()

(inlLen1, inlDAC1, inlADC1) = inl1
(inlLen2, inlDAC2, inlADC2) = inl2

# build splines for each channel
log.info("Building splines for %s"%inlPath1)
inlSplines1 = zachUtil.build_inl_splines(inl1, inlTwrs1)
(adc2dac1Splines, dac2adc1Splines) = inlSplines1

log.info("Building splines for %s"%inlPath1)
inlSplines2 = zachUtil.build_inl_splines(inl2, inlTwrs2)
(adc2dac2Splines, dac2adc2Splines) = inlSplines2

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "inlDiff(%s,%s)"%(inlPath1,inlPath2))

# gobal summary histogram
resid_summary = ROOT.TH1I("resid_summary",
                          "resid_summary",
                          100,0,0)

                
# calc diffs for each channel
for twr in inlTwrs1:
    log.info("Processing TEM=%d"%twr)
    for lyr in range(calConstant.NUM_LAYER):
        # calCalibXML uses 'row' indexing, not layer
        row = calCalibXML.layerToRow(lyr)
        for col in range(calConstant.NUM_FE):
            for face in range(calConstant.NUM_END):
                online_face = zachUtil.offline_face_to_online[face]

                for rng in range(calConstant.NUM_RNG):
                    # pick 1st channel to saturate as i don't want to compare after saturation
                    length1   = int(inlLen1[rng][twr,row,online_face,col])
                    length2   = int(inlLen2[rng][twr,row,online_face,col])

                    test_adc1 = inlADC1[rng][twr,row,online_face,col,0:length1]
                    test_adc2 = inlADC2[rng][twr,row,online_face,col,0:length2]

                    adc_max1 = test_adc1[-1]
                    adc_max2 = test_adc2[-1]

                    if (adc_max1 < adc_max2):
                        length = length1
                        test_adc = test_adc1
                    else:
                        length = length2
                        test_adc = test_adc2
                    

                    adc2dac1 = adc2dac1Splines[(twr,row,online_face,col,rng)]
                    adc2dac2 = adc2dac2Splines[(twr,row,online_face,col,rng)]


                    ### INIT  ROOT HISTS ###
                    channel_str = "%d_%d_%d_%d_%d"%(twr,lyr,col,face,rng)
                    diffHist = ROOT.TH1S("inlDiff_%s"%channel_str,
                                         "inlDiff_%s"%channel_str,
                                         100,0,0)
                    ### PLOT ARRAYS ###
                    x = array.array('f')
                    resid = array.array('f')
                    
                    for adc in test_adc:
                        dac1 = adc2dac1.Eval(adc)
                        dac2 = adc2dac2.Eval(adc)

                        ### POPULATE PLOTS & HISTS ###
                        x.append(adc)
                        diff = dac2 - dac1
                        print channel_str, adc, dac1, dac2, diff
                        resid.append(diff)
                        diffHist.Fill(diff)
                        resid_summary.Fill(diff)

                    ### INIT PLOTS ###

                    # residual plot
                    gResid = ROOT.TGraph(length, x, resid)
                    gResid.SetNameTitle("inlResid_%s"%channel_str,
                                        "inlResid_%s"%channel_str)
                                        
                    cResid = ROOT.TCanvas("inlResid_%s"%channel_str,
                                          "inlResid_%s"%channel_str,-1)
                    
                    gResid.Draw("AL*")
                    cResid.Write()

log.info("Writing %s"%rootPath)
rootFile.Write()
rootFile.Close()
                                     


                
                        

                        

                    

                        

                        
                    
                                

