
"""
Tool to generate CAL adc2nrg XML file from asymmetry, cidac2adc & mevPerDAC xml files

Usage:
genADC2NRG cidac2adc.xml asym.xml mpd.xml adc2nrg.xml

where:
    cidac2adc.xml - input intNonlin calibration file
    asym.xml      - input asymmetry calibration file
    mpd.xml       - input MevPerDAC calibration file
    adc2nrg.xml   - output adc2nrg calibration file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL ADC2NRG calibration XML files from asymmetry, cidac2adc & mevPerDAC xml files"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/08/03 13:11:03 $"
__version__   = "$Revision: 1.4 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import sets
import array
import math
import ROOT

import calDacXML
import calCalibXML
import calConstant
import zachUtil

# most probably mev of muon mip vertically through cal xtal
MUON_PEAK_ENERGY = 11.2

#######################################################################################

if __name__ == '__main__':

    # constants
    usage      = "genADC2NRG cidac2adc.xml asym.xml mpd.xml adc2nrg.xml"                                                                                                                                                     
    nTXTFields = 7

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('genADC2NRG')
    log.setLevel(logging.INFO)


    # get environment settings
    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    

    # parse commandline
    #  - code stolen from: http://python.active-venture.com/lib/module-getopt.html
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.exception("getopt exception: "+usage)
        sys.exit(-1)

    if (len(args) != 4):
        log.error("Need 4 filenames: " + usage)
        sys.exit(1)

    (inlPath, asymPath, mpdPath, adc2nrgPath) = args

    ### read input files ###

    # open and read XML intNonlin file
    xmlFile = calCalibXML.calIntNonlinCalibXML(inlPath)
    inlData = xmlFile.read()
    (lenData, dacData, adcData) = inlData
    towers = xmlFile.getTowers()
    xmlFile.close()
    log.info("Building inl splines")
    inlSplines = zachUtil.build_inl_splines(inlData, towers)
    (adc2dac, dac2adc) = inlSplines


    # open and read XML Asymmetry file
    xmlFile = calCalibXML.calAsymCalibXML(asymPath)
    (xpos, asymData) = xmlFile.read()
    if (towers != xmlFile.getTowers()):
        log.error("asym towers don't match inl towers")
        sys.exit(1)
    xmlFile.close()
    asymSplines = zachUtil.build_asym_splines((xpos, asymData), towers)
    (pos2asym, asym2pos) = asymSplines

    # open and read XML MevPerDAC file
    xmlFile = calCalibXML.calMevPerDacCalibXML(mpdPath)
    mpdData = xmlFile.read()
    if (towers != xmlFile.getTowers()):
        log.error("mpd towers don't match inl towers")
        sys.exit(1)
    xmlFile.close()


    # define output array
    outData = Numeric.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_END,
                             calConstant.NUM_FE,
                             2),
                            Numeric.Float32)
    slopeIdx = 0 # index into outData last field
    errIdx   = 1 # index into outData last field

    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_ROW):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for diode in range(calConstant.NUM_DIODE):
                        rng = diode*2 # only process X8 adc channels

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print twr, lyr, col, face, diode
                            print "OS version", os.name
                            print "Python version", sys.version
                            print "ROOT version", ROOT.gROOT.GetVersion()

                        # find 'overall' light asymmetry @ center of xtal
                        p2a = pos2asym[(twr,row,col,diode)]
                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print (twr,row,col,diode)
                            print p2a.GetTitle()
#                             for n  in range(10):
#                                 kx = 2.0
#                                 ky = 2.0
#                                 p2a.GetKnot(n,kx,ky)
#                                 print kx, ky

                        asym_ctr = p2a.Eval(0)
                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " asym_ctr: ", asym_ctr
                        # need to split ratio up as I am applying it equally to
                        # both ends
                        # 0.25 would nomrally be 0.5
                        asym_ctr /= 4

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " asym_ctr/4: ", asym_ctr

                        # move asymmetry out of log space
                        asym_ctr = math.exp(asym_ctr)

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " exp: ", asym_ctr

                        mpd = mpdData[twr,row,col,diode]
                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " mpd: " , mpd

                        muon_peak_dac = MUON_PEAK_ENERGY / mpd

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " peak_dac: " , muon_peak_dac

                        if face == calConstant.OFFLINE_FACE_POS:
                            muon_peak_dac *= asym_ctr
                        else:
                            muon_peak_dac /= asym_ctr

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " asym_corr_peak: " , muon_peak_dac

                        d2a = dac2adc[(twr,row,online_face,col,rng)]
                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print d2a.GetTitle()
                            kx = 0.0
                            ky = 1.0
                            length = lenData[rng][twr, row, online_face, col, 0]
                            print id(kx), id(ky)
                            for n in range(length):
                                d2a.GetKnot(n,kx,ky)
                                print kx, dacData[rng][twr,row,online_face,col,n], \
                                      ky, adcData[rng][twr,row,online_face,col,n]

                        
                        adc = d2a.Eval(muon_peak_dac)

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " adc_peak: ", adc

                        adc2nrg = MUON_PEAK_ENERGY / adc

                        if (twr == 9 and lyr == 3 and col == 4 and face == 0 and diode == 1):
                            print " adc2nrg: ", adc2nrg


                        outData[twr, row, online_face, col, diode] = adc2nrg

                        #print twr, lyr, col, face, diode, adc2nrg

    log.info('Writing output file %s', adc2nrgPath)

    outFile = calDacXML.calEnergyXML(adc2nrgPath, 'adc2nrg', calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = towers)
    outFile.close()


    sys.exit(0)                            
