"""
Tool to generate CAL adc2nrg and muSlope XML file from asymmetry, cidac2adc & mevPerDAC xml files

Usage:
genADC2NRG cidac2adc.xml asym.xml mpd.xml adc2nrg.xml muSlope.xml

where:
    cidac2adc.xml - input intNonlin calibration file
    asym.xml      - input asymmetry calibration file
    mpd.xml       - input MevPerDAC calibration file
    adc2nrg.xml   - output adc2nrg calibration file
    muSlope.xml   - output muSlope calibration file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL ADC2NRG and muSlope calibration XML files from asymmetry, cidac2adc & mevPerDAC xml files"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:49 $"
__version__   = "$Revision: 1.6 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import numarray
import sets
import array
import math
import ROOT

import calDacXML
import calCalibXML
import calConstant
import cgc_util

# most probably mev of muon mip vertically through cal xtal
MUON_PEAK_ENERGY = 11.2

# get environment settings
try:
    calibUtilRoot = os.environ['CALIBUTILROOT']
except:
    log.error('CALIBUTILROOT must be defined')
    sys.exit(1)    

dtdName    = "calCalib_v2r3.dtd" #default value
dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)


#######################################################################################

if __name__ == '__main__':

    # constants
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
        log.exception("getopt exception: "+__doc__)
        sys.exit(-1)

    if (len(args) != 5):
        log.error("Need 5 filenames: " + __doc__)
        sys.exit(1)

    (inlPath, asymPath, mpdPath, adc2nrgPath, muSlopePath) = args

    ### read input files ###

    # open and read XML intNonlin file
    xmlFile = calCalibXML.calIntNonlinCalibXML(inlPath)
    inlData = xmlFile.read()
    (lenData, dacData, adcData) = inlData
    towers = xmlFile.getTowers()
    xmlFile.close()
    log.info("Building inl splines")
    inlSplines = cgc_util.build_inl_splines(inlData, towers)
    (adc2dac, dac2adc) = inlSplines


    # open and read XML Asymmetry file
    xmlFile = calCalibXML.calAsymCalibXML(asymPath)
    (xpos, asymData) = xmlFile.read()
    if (towers != xmlFile.getTowers()):
        log.error("asym towers don't match inl towers")
        sys.exit(1)
    xmlFile.close()
    asymSplines = cgc_util.build_asym_splines((xpos, asymData), towers)
    (pos2asym, asym2pos) = asymSplines

    # open and read XML MevPerDAC file
    xmlFile = calCalibXML.calMevPerDacCalibXML(mpdPath)
    mpdData = xmlFile.read()
    if (towers != xmlFile.getTowers()):
        log.error("mpd towers don't match inl towers")
        sys.exit(1)
    xmlFile.close()


    # define output array(s)
    adc2nrgData = numarray.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_END,
                             calConstant.NUM_FE,
                             2),
                            numarray.Float32)
    muSlopeData = numarray.zeros((calConstant.NUM_TEM,
                                 calConstant.NUM_ROW,
                                 calConstant.NUM_END,
                                 calConstant.NUM_FE,
                                 calConstant.NUM_RNG,
                                 2),
                                numarray.Float32)
    slopeIdx = 0 # index into adc2nrgData last field
    errIdx   = 1 # index into adc2nrgData last field

    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for diode in range(calConstant.NUM_DIODE):
                        # find 'overall' light asymmetry @ center of xtal
                        p2a = pos2asym[(twr,row,col,diode)]

                        asym_ctr = p2a.Eval(0)

                        # need to split ratio up as I am applying it equally to
                        # both ends
                        # 0.25 would nomrally be 0.5
                        asym_ctr /= 4

                        # move asymmetry out of log space
                        asym_ctr = math.exp(asym_ctr)

                        mpd = mpdData[twr,row,col,diode]

                        muon_peak_dac = MUON_PEAK_ENERGY / mpd

                        if face == calConstant.OFFLINE_FACE_POS:
                            muon_peak_dac *= asym_ctr
                        else:
                            muon_peak_dac /= asym_ctr

                        # now loop through both adc ranges in single diode
                        for thx in range(2):
                            rng = diode*2+thx

                            d2a = dac2adc[(twr,row,online_face,col,rng)]
                        
                            adc = d2a.Eval(muon_peak_dac)

                            adc2nrg = MUON_PEAK_ENERGY / adc

                            # fill in muSlope array
                            muSlopeData[twr, row, online_face, col, rng, slopeIdx] = adc2nrg

                           # print twr, lyr, col, face, rng, adc2nrg

                            # fill in adc2nrg array
                            if thx == 0:
                                adc2nrgData[twr, row, online_face, col, diode] = adc2nrg

    log.info('Writing output file %s', adc2nrgPath)

    outFile = calDacXML.calEnergyXML(adc2nrgPath, 'adc2nrg', calCalibXML.MODE_CREATE)
    outFile.write(adc2nrgData, tems = towers)
    outFile.close()


    log.info('Writing output file %s', muSlopePath)

    outFile = calCalibXML.calMuSlopeCalibXML(muSlopePath, calCalibXML.MODE_CREATE)
    outFile.write(muSlopeData, tems = towers)
    outFile.close()

    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(muSlopePath, dtdPath)


    sys.exit(0)                            
