"""
Force linear dac scale over portions of cidac2adc curve where cidac is non-linear.

linearDAC input.xml output.xml
where:
    input.xml intNonlin XML file with following characteristics:
        - should have _all_ points from test (no grouping) aka 'adcmeans' file
        - should have adc scale pedestal subtracted.
    output.xml output, linearized, intNonlin XML file
"""

__facility__  = "Offline"
__abstract__  = "Force linear dac scale over portions of cidac2adc curve where cidac is non-linear."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/01/24 16:36:21 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import ROOT
import array
import itertools

import calCalibXML
import calConstant
import zachUtil

# get environment settings
try:
    calibUtilRoot = os.environ['CALIBUTILROOT']
except:
    log.error('CALIBUTILROOT must be defined')
    sys.exit(1)
dtdName = "calCalib_v2r3.dtd" #default value
dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

# list of CIDAC values probed in order during singlex16 test
CIDAC_TEST_VALS = \
    [ \
             0,    2,     4,      6,      8,      10,      12,      14,      16,      18,     20,    22,    24,    26,   28,   30, 32, \
             34,   36,    38,     40,     42,     44,      46,      48,      50,      52,     54,    56,    58,    60,   62,   64,\
             80,   96,    112,    128,    144,    160,     176,     192,     208,     224,    240,   256,   272,   288,\
             304,  320,   336,    352,    368,    384,     400,     416,     432,     448,    464,   480,   496,   512,\
             543,  575,   607,    639,    671,    703,     735,     767,     799,     831,    863,   895,   927,   959,\
             991,  1023,  1055,   1087,   1119,   1151,    1183,    1215,    1247,    1279,\
             1311, 1343,  1375,   1407,   1439,   1471,    1503,    1535,    1567,    1599,\
             1631, 1663,  1695,   1727,   1759,   1791,    1823,    1855,    1887,    1919,\
             1951, 1983,  2015,   2047,   2079,   2111,    2143,    2175,    2207,    2239,\
             2271, 2303,  2335,   2367,   2399,   2431,    2463,    2495,    2527,    2559,\
             2591, 2623,  2655,   2687,   2719,   2751,    2783,    2815,    2847,    2879,\
             2911, 2943,  2975,   3007,   3039,   3071,    3103,    3135,    3167,    3199,\
             3231, 3263,  3295,   3327,   3359,   3391,    3423,    3455,    3487,    3519,\
             3551, 3583,  3615,   3647,   3679,   3711,    3743,    3775,    3807,    3839,\
             3871, 3903,  3935,   3967,   3999,   4031,    4063,    4095
             ]

# lex1,hex1 below xxx_start is extrapolated from
# straight line fit through [start,end]
# indexed by rng num
DAC_LINEFIT_START = (64,  64, 64, 64)
LINEFIT_IDX_START = tuple([CIDAC_TEST_VALS.index(x) for x in DAC_LINEFIT_START])

DAC_LINEFIT_END = (192, 192, 192, 192)
LINEFIT_IDX_END = tuple(CIDAC_TEST_VALS.index(x) for x in DAC_LINEFIT_END)


#######################################################################################
if __name__ == '__main__':

    # constants
    usage      = "linearDAC input.xml output.xml"

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('linearDAC')
    log.setLevel(logging.INFO)

    # check command line
    try:
        # currently no args supported
        (optList,args) = getopt.getopt(sys.argv[1:],"")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    if len(args) != 2:
        log.error("Wrong # of paramters: %s"%usage)
        sys.exit(1)

    (inPath, outPath) = args

    # read in data files.
    log.info("Reading input %s"%inPath)
    inFile = calCalibXML.calIntNonlinCalibXML(inPath)
    inData = inFile.read()
    (inLen, inDAC, inADC) = inData
    inTwrSet = inFile.getTowers()

    #create output arrays
    outLen = list()
    outDAC = list()
    outADC = list()
    for rng in range(calConstant.NUM_RNG):
        outLen.append(inLen[rng].copy())
        outDAC.append(inDAC[rng].copy())
        outADC.append(inADC[rng].copy())

    # create temp ROOT file
    (basename, ext) = os.path.splitext(outPath)
    rootPath = basename + ".linearDAC.root"
    rootFile = ROOT.TFile(rootPath,
                          "RECREATE")

    for twr in inTwrSet:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]

                    for rng in range(calConstant.NUM_RNG):

                        #########################################
                        # (X1) # extrapolate below 500 ADC ####
                        #########################################

                        ### RETRIEVE CHANNEL DATA ###
                        channel_str = "cidac2adc_%s_%s_%s_%s_%s"%(twr, lyr, col, face, rng)
                        length = inLen[rng][twr,row,online_face,col]

                        # skip empty channels
                        if length < 2:
                            continue

                        dac = inDAC[rng][twr,row,online_face,col,0:length].copy()
                        adc = inADC[rng][twr,row,online_face,col,0:length].copy()

                        #print "start"
                        #print channel_str
                        #print length
                        #print dac
                        #print adc

                        ### CURVE SATURATION ###
                        # clip saturated portion of curve
                        max_adc = adc[-1]
                        length = len(list(itertools.takewhile(lambda x:x < max_adc*.99, adc)))
                        adc = adc[0:length]
                        dac = dac[0:length]

                        #print "saturate"
                        #print length
                        #print dac
                        #print adc

                        # extrapolate end of curve to last point of saturation
                        # - setup points for linear extrapolation
                        dac1 = dac[-2]
                        dac2 = dac[-1]
                        adc1 = adc[-2]
                        adc2 = adc[-1]
                        max_dac = zachUtil.linear_extrap(adc1, adc2, max_adc, dac1, dac2)

                        adc = Numeric.concatenate((adc, [max_adc]))
                        dac = Numeric.concatenate((dac, [max_dac]))
                        length += 1

                        #print "extrap"
                        #print length
                        #print dac
                        #print adc

                        ### LINEAR FIT ###

                        # find points in range for fit
                        fit_start = LINEFIT_IDX_START[rng]
                        fit_end   = LINEFIT_IDX_END[rng]
                        fit_dacs = dac[fit_start:fit_end+1]
                        fit_adcs = adc[fit_start:fit_end+1]

                        # create ROOT graph for fitting line
                        g = ROOT.TGraph(len(fit_dacs),
                                        array.array('f',fit_adcs),
                                        array.array('f',fit_dacs))
                        c = ROOT.TCanvas(channel_str, channel_str)
                        g.SetNameTitle(channel_str, channel_str)
                        g.Fit('pol1',"Q")
                        f = g.GetFunction('pol1')
                        g.Draw("A*")
                        c.Write()

                        #print "fit"
                        #print fit_dacs
                        #print fit_adcs


                        #                         ### REMOVE NONLINEAR PORTION ###
                        #                         # remove below dac = 256 #
                        #                         remove_range = Numeric.greater_equal(dac, DAC_LINEFIT_START[rng])
                        #                         dac = Numeric.compress(remove_range, dac)
                        #                         adc = Numeric.compress(remove_range, adc)

                        #### DAC SCALE PEDESTAL CORRECTION ###
                        # retrieve dac scale pedestal
                        x1_dac_ped = f.GetParameter(0)
                        slope = f.GetParameter(1)
                        dac = dac - x1_dac_ped

                        #print "ped"
                        #print length
                        #print dac
                        #print adc

                        # switch x & y axis to go from adc2dac ->dac2adc
                        slope = 1/slope

                        # fill in low end of curve w/ new straight line
                        adc[0:fit_end] = (dac[0:fit_end]*slope).astype(Numeric.Float32)

                        #print "linearize"
                        #print length
                        #print dac
                        #print adc


                        # remove portions of curve w/ negative dac
                        mask = Numeric.greater_equal(dac,0)
                        dac = Numeric.compress(mask,dac)
                        adc = Numeric.compress(mask,adc)
                        length = len(dac)

                        #print "clip_zero"
                        #print length
                        #print dac
                        #print adc

                        
                        
                        # insert 0,0 point @ beginning of curve
                        if dac[0] != 0:
                            dac = Numeric.concatenate(([0],dac))
                            adc = Numeric.concatenate(([0],adc))
                            length += 1

                        #print "zero"
                        #print length
                        #print dac
                        #print adc

                        # write channel data back out to main data array
                        outDAC[rng][twr,row,online_face,col,0:length] = dac[:].astype(Numeric.Float32)
                        outADC[rng][twr,row,online_face,col,0:length] = adc[:].astype(Numeric.Float32)
                        outLen[rng][twr,row,online_face,col] = length

                        #print "output"
                        #print outDAC[rng][twr,row,online_face,col,0:length] 
                        #print outADC[rng][twr,row,online_face,col,0:length]
                        #print outLen[rng][twr,row,online_face,col]

                        msg = " ".join([str(x) for x in channel_str, 
                                       "n_pts=",   length, 
                                       "max_adc=", max_adc, 
                                       "max_dac=", max_dac, 
                                       "slope=",   slope,
                                       "dac_ped=", x1_dac_ped])
                        log.info(msg)



    ## write output file ##
    log.info("Building new inl file: %s"%outPath)
    outFile = calCalibXML.calIntNonlinCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(outLen,outDAC,outADC,tems=inTwrSet)
    outFile.close()

    calCalibXML.insertDTD(outPath, dtdPath)

    rootFile.Write()
    rootFile.Close()

    sys.exit(0)
