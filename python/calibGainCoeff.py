"""
Read in 2 intNonlin text files (presumably taken w/ different calibGain settings) and dump text file w/ the ratios between each HEX8 channel at 1500 adc units.
This ratio will represent the effect of the calibGain setting, the ratio is format (CIDAC_calibGainOFF/CIDAC_calibGainON)

Note: calibGainCoeff attempts to autodetect if input data is for partial LAT only (<16 towers).

calibGainCoeff inl_calibGainOn.xml inl_calibGainOff.xml

where:
    inl_calibGainOn.xml  = input intNonlin Cal offline xml file
    inl_calibGainOff.xml = input intNonlin Cal offline xml file

"""

__facility__  = "Offline"
__abstract__  = "Tool to extract effect of calibGain setting from two intNonlin XML files"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/08/17 16:35:28 $"
__version__   = "$Revision: 1.8 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import sets
import array
import ROOT

import calCalibXML
import calConstant
import cgc_util

#######################################################################################
if __name__ == '__main__':

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('calibGainCoeff')
    log.setLevel(logging.INFO)

    if (len(sys.argv[1:]) != 2):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)

    cgOnPath  = sys.argv[1]
    cgOffPath = sys.argv[2]

    # read in data files.
    log.info("Reading %s"%cgOnPath)
    xmlOnFile = calCalibXML.calIntNonlinCalibXML(cgOnPath)
    cgOnData = xmlOnFile.read()
    (cgOnLen, cgOnDAC, cgOnADC) = cgOnData
    cgOnTwrSet = xmlOnFile.getTowers()

    log.info("Reading %s"%cgOffPath)
    xmlOffFile = calCalibXML.calIntNonlinCalibXML(cgOffPath)
    cgOffData = xmlOffFile.read()
    (cgOffLen, cgOffDAC, cgOffADC) = cgOffData
    cgOffTwrSet = xmlOffFile.getTowers()

    log.info("Building inl splines")
    cgOnSplines = cgc_util.build_inl_splines(cgOnData, cgOnTwrSet)
    cgOffSplines = cgc_util.build_inl_splines(cgOffData, cgOffTwrSet)

    (adc2dacOn, dac2adcOn) = cgOnSplines
    (adc2dacOff, dac2adcOff) = cgOffSplines
    
    log.info("Evalutating ratios")
    # calc & print calibGainFactor for each channel
    for twr in cgOnTwrSet:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]

                    # HEX8 only
                    rng = 2
                    cgOnSpline = adc2dacOn[(twr,row,online_face,col,rng)]
                    cgOffSpline = adc2dacOff[(twr,row,online_face,col,rng)]

                    # evaluate both splines @ 1500 ADC units
                    onVal = cgOnSpline.Eval(1500)
                    offVal = cgOffSpline.Eval(1500)
                    ratio =  offVal/onVal

                    
                    print ' '.join([str(x) for x in twr, lyr, col, face, ratio])


