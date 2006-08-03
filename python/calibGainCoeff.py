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
__date__      = "$Date: 2006/06/22 20:56:57 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
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
import zachUtil

#######################################################################################
if __name__ == '__main__':

    # constants
    usage      = "calibGainCoeff inl_calibGainOn.xml inl_calibGainOff.xml"

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('calibGainCoeff')
    log.setLevel(logging.INFO)

    if (len(sys.argv[1:]) != 2):
        log.error("Need 2 filenames: " + usage)
        sys.exit(1)

    cgOnPath   = sys.argv[1]
    cgOffPath = sys.argv[2]

    # read in data files.
    xmlOnFile = calCalibXML.calIntNonlinCalibXML(cgOnPath)
    (cgOnLen, cgOnDAC, cgOnADC) = xmlOnFile.read()
    cgOnTwrSet = xmlOnFile.getTowers()

    xmlOffFile = calCalibXML.calIntNonlinCalibXML(cgOffPath)
    (cgOffLen, cgOffDAC, cgOffADC) = xmlOffFile.read()
    cgOffTwrSet = xmlOffFile.getTowers()

    
    # calc & print calibGainFactor for each channel
    for twr in cgOnTwrSet:
        for lyr in range(8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(12):
                for face in range(2):
                    online_face = zachUtil.offline_face_to_online[face]

                    rng = 2 # HEX8 only
                    onLen  = int(cgOnLen[rng][twr,row,online_face,col])
                    offLen  = int(cgOffLen[rng][twr,row,online_face,col])

                    # build arrays for each channel
                    cgOnDACArray = array.array('d',  cgOnDAC[rng][twr,row,online_face,col,0:onLen].tolist()   )
                    cgOnADCArray = array.array('d',  cgOnADC[rng][twr,row,online_face,col,0:onLen].tolist()   )
                    cgOffDACArray = array.array('d', cgOffDAC[rng][twr,row,online_face,col,0:offLen].tolist() )
                    cgOffADCArray = array.array('d', cgOffADC[rng][twr,row,online_face,col,0:offLen].tolist() )

                    # build splines (ADC->DAC) direction
                    cgOnSpline = ROOT.TSpline3("%d_%d_%d_%d_cgOn"%(twr,lyr,col,face),
                                               cgOnADCArray, cgOnDACArray,
                                               onLen)

                    cgOffSpline = ROOT.TSpline3("%d_%d_%d_%d_cgOff"%(twr,lyr,col,face),
                                               cgOffADCArray, cgOffDACArray,
                                               offLen)

                    # evaluate both splines @ 1500 ADC units
                    onVal = cgOnSpline.Eval(1500)
                    offVal = cgOffSpline.Eval(1500)
                    ratio =  offVal/onVal

                    
                    print ' '.join([str(x) for x in twr, lyr, col, face, ratio])


