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
__date__        = "$Date: 2008/05/22 16:50:20 $"
__version__     = "$Revision: 1.9 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calCalibXML
import calConstant
import logging
import cgc_util
import array


# setup logger

logging.basicConfig()
log = logging.getLogger('inlDiff')
log.setLevel(logging.INFO)

# check command line
if len(sys.argv) != 4:
    log.error("%d args found."%len(sys.argv))
    log.error(__doc__)
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
    continue

# load up arrays
log.info("Reading %s"%inlPath1)
inl1 = inlFile1.read()
log.info("Reading %s"%inlPath2)
inl2 = inlFile2.read()

(inlLen1, inlDAC1, inlADC1) = inl1
(inlLen2, inlDAC2, inlADC2) = inl2

# build splines for each channel
log.info("Building splines for %s"%inlPath1)
inlSplines1 = cgc_util.build_inl_splines(inl1, inlTwrs1)
(adc2dac1Splines, dac2adc1Splines) = inlSplines1

log.info("Building splines for %s"%inlPath2)
inlSplines2 = cgc_util.build_inl_splines(inl2, inlTwrs2)
(adc2dac2Splines, dac2adc2Splines) = inlSplines2

# set up pyROOT
import ROOT
ROOT.gROOT.Reset()
log.info("Opening %s"%rootPath)
rootFile = ROOT.TFile(rootPath,
                      "recreate",
                      "inlDiff(%s,%s)"%(inlPath1,inlPath2))

# gobal summary histogram
resid_summary = []
resid_sum_prof = []
for rng in range(calConstant.NUM_RNG):
    resid_summary.append(ROOT.TH2S("resid_summary_%s"%rng,
                                   "resid_summary_%s"%rng,
                                   4096,0,4096,
                                   8192,-4096,4096))

    resid_sum_prof.append(ROOT.TProfile("resid_sum_prof_%s"%rng,
                                        "resid_sum_prof_%s"%rng,
                                        400,0,4096))
                
# calc diffs for each channel
for twr in inlTwrs1:
    log.info("Processing TEM=%d"%twr)
    for lyr in range(calConstant.NUM_LAYER):
        # calCalibXML uses 'row' indexing, not layer
        row = calCalibXML.layerToRow(lyr)
        for col in range(calConstant.NUM_FE):
            for face in range(calConstant.NUM_END):
                online_face = calConstant.offline_face_to_online[face]

                for rng in range(calConstant.NUM_RNG):
                    # pick 1st channel to saturate as i don't want to compare after saturation
                    length1   = int(inlLen1[rng][twr,row,online_face,col])
                    length2   = int(inlLen2[rng][twr,row,online_face,col])

                    # skip empty channels
                    if (length1 < 2 and length2 < 2):
                        continue

                    # error condition if channel is missing from only one input file
                    if (length1 <2 or length2 <2):
                        log.error("Missing channel in one file: " + str([twr,lyr,row,col,face,rng]))

                    test_dac1 = inlDAC1[rng][twr,row,online_face,col,0:length1]
                    test_dac2 = inlDAC2[rng][twr,row,online_face,col,0:length2]

                    dac_max1 = test_dac1[-1]
                    dac_max2 = test_dac2[-1]

                    if (dac_max1 < dac_max2):
                        length = length1
                        test_dac = test_dac1
                    else:
                        length = length2
                        test_dac = test_dac2
                    
                    dac2adc1 = dac2adc1Splines[(twr,row,online_face,col,rng)]
                    dac2adc2 = dac2adc2Splines[(twr,row,online_face,col,rng)]

                    ### INIT  ROOT HISTS ###
                    channel_str = "%d_%d_%d_%d_%d"%(twr,lyr,col,face,rng)
                    diffHist = ROOT.TH1S("inlDiff_%s"%channel_str,
                                         "inlDiff_%s"%channel_str,
                                         100,0,0)
                    ### PLOT ARRAYS ###
                    x = array.array('f')
                    resid = array.array('f')
                    
                    for dac in test_dac:
                        adc1 = dac2adc1.Eval(dac)
                        adc2 = dac2adc2.Eval(dac)

                        ### POPULATE PLOTS & HISTS ###
                        x.append(dac)
                        diff = adc2 - adc1
                        resid.append(diff)
                        diffHist.Fill(diff)
                        resid_summary[rng].Fill(dac,diff)
                        resid_sum_prof[rng].Fill(dac,diff)

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
                                     


                
                        

                        

                    

                        

                        
                    
                                


