"""
Apply LEdiode->HEdiode crosstalk correction to cidac2adc xml file.

Algorithm is as follows:
- Read in 2 intNonlin text files, one with full cidac2adc curves and another with measured cross-diode crosstalk.
- extrapolate xtalk across full HE range apply it to the input cidac2adc curves.
- output new cidac2adc xml file.

Note: xdiodeXtalk attempts to autodetect if input data is for partial LAT only (<16 towers).

xdiodeXtalk <FLIGHT_GAIN|MUON_GAIN> cidac2adc.xml xtalk.xml output.xml
where:
     -m            muon gain mode
     cidac2adc.xml input intNonlin XML file
     xtalk.xml     intNonlin XML file contains LE->HE crosstalk values in HE channels.
     output.xml    output, xtalk corrected, intNonlin XML file


"""

__facility__  = "Offline"
__abstract__  = "Tool to apply cross-diode crosstalk correction to intNonlin XML files"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/01/16 17:22:14 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
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

import pdb

# CONSTANTS #
XTALK_FACTOR_FLIGHT_GAIN = 1.0/5.5
XTALK_FACTOR_MUON_GAIN   = 1.7
# dac value to extrapolate to (before I divide by XTALK_FACTOR)
extrapDACTo = 4095/XTALK_FACTOR_FLIGHT_GAIN
# we're extrapolating a pretty long line @ the end.
# we want to specify points in 50 dac unit steps when we're done
# or the cubic splines may go awry
extrapPitch = 50/XTALK_FACTOR_FLIGHT_GAIN

# CFG VARS #
muonGain     = False
XTALK_FACTOR = XTALK_FACTOR_FLIGHT_GAIN


# get environment settings
try:
    calibUtilRoot = os.environ['CALIBUTILROOT']
except:
    log.error('CALIBUTILROOT must be defined')
    sys.exit(1)
dtdName = "calCalib_v2r3.dtd" #default value
dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)


#######################################################################################
if __name__ == '__main__':

    # constants
    usage      = "xdiodeXtalk <FLIGHT_GAIN|MUON_GAIN> cidac2adc.xml xtalk.xml output.xml"

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('xdiodeXtalk')
    log.setLevel(logging.INFO)

    # check command line
    try:
        # currently no args supported
        (optList,args) = getopt.getopt(sys.argv[1:],"")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    if len(args) != 4:
        log.error("Wrong # of paramters: %s"%usage)
        sys.exit(1)

    (gain,inPath,xtalkPath,outPath) = args

    if gain == "MUON_GAIN":
        muonGain = True
        XTALK_FACTOR = XTALK_FACTOR_MUON_GAIN
    elif gain != "FLIGHT_GAIN":
        log.error("Invalid gain string: %s"%gain)
        sys.exit(1)
    
    # read in data files.
    log.info("Reading input %s"%inPath)
    inFile = calCalibXML.calIntNonlinCalibXML(inPath)
    inData = inFile.read()
    (inLen, inDAC, inADC) = inData
    inTwrSet = inFile.getTowers()

    log.info("Reading xtalk %s"%xtalkPath)
    xtalFile = calCalibXML.calIntNonlinCalibXML(xtalkPath)
    xtalkData = xtalFile.read()
    (xtalkLen, xtalkDAC, xtalkADC) = xtalkData
    xtalkTwrSet = xtalFile.getTowers()

    if (xtalkTwrSet != inTwrSet):
        log.error("intNonlin file and xtalk file have different CAL modules!")
        sys.exit(-1)

    log.info("Building xtalk splines")
    xtalkSplines = zachUtil.build_inl_splines(xtalkData, xtalkTwrSet)
    (adc2dacXtalk, dac2adcXtalk) = xtalkSplines

    # clip length of dac curve in muon gain
    # maximum dac value that will remain on scale after muon gain factor is applied
    #pdb.set_trace()
    max_dac_muon = 4095/XTALK_FACTOR_MUON_GAIN
    if muonGain:
        log.info("Clipping xtalk splines")
        for twr in inTwrSet:
            for lyr in range(calConstant.NUM_LAYER):
                # calCalibXML uses 'row' indexing, not layer
                row = calCalibXML.layerToRow(lyr)
                for col in range(calConstant.NUM_FE):
                    for face in range(calConstant.NUM_END):
                        online_face = calConstant.offline_face_to_online[face]
                        for rng in range(2,4):
                            # find 1st point which will saturate dac scale after normalizing
                            saturated = xtalkDAC[rng][twr,row,online_face,col] > max_dac_muon
                            try:
                                index = saturated.tolist().index(1)
                                # length only reset if index() does not throw exception
                                xtalkLen[rng][twr,row,online_face,col,0] = index
                            except ValueError:
                                # do nothing here.... wish i knew a better way
                                index = None
                            #print "index",index,"len",xtalkLen[rng][twr,row,online_face,col,0]

                            # extrapolate last point to max_dac_muon
                            # we will only add an extra point if there is space left in the array
                            length = xtalkLen[rng][twr,row,online_face,col,0]
                            if length < len(saturated):
                                #print "adding extra point"
                                # only add an extra point if last good point is < max_dac
                                if xtalkDAC[rng][twr,row,online_face,col,length-1] < max_dac_muon:
                                    length = length + 1
                                    index = length - 1
                                    xtalkLen[rng][twr,row,online_face,col,0] = length
                                    xtalkDAC[rng][twr,row,online_face,col,index] = max_dac_muon
                                    
                                    adc1 = xtalkADC[rng][twr,row,online_face,col,index-2]
                                    adc2 = xtalkADC[rng][twr,row,online_face,col,index-1]
                                    dac1 = xtalkDAC[rng][twr,row,online_face,col,index-2]
                                    dac2 = xtalkDAC[rng][twr,row,online_face,col,index-1]
                                    dac3 = max_dac_muon
                                    
                                    xtalkADC[rng][twr,row,online_face,col,index] = zachUtil.linear_extrap(dac1, dac2, dac3,
                                                                                                          adc1, adc2)
                                                                        

    # only need to extrapolate dac scale if we are shortening it, such as in flight gain
    else:
        log.info("Extrapolating xtalk splines")
        for twr in inTwrSet:
            for lyr in range(8):
                # calCalibXML uses 'row' indexing, not layer
                row = calCalibXML.layerToRow(lyr)
                for col in range(12):
                    for face in range(2):
                        online_face = calConstant.offline_face_to_online[face]
                        for rng in range(2,4):

                            xtalkSpline = dac2adcXtalk[(twr,row,online_face,col,rng)]

                            xtalk3500 = xtalkSpline.Eval(3500)
                            xtalk4095 = xtalkSpline.Eval(4095)

                            # append points every 20*FACTOR dac units up to extrapDACTo along linear extrapolated path
                            # in order to keep spline from getting 'creative' over a large jump
                            newDAC = 4095 + extrapPitch
                            while (newDAC < extrapDACTo):
                                newADC = zachUtil.linear_extrap(3500, 4095, newDAC,
                                                              xtalk3500, xtalk4095)
                                xtalkLen[rng][twr,row,online_face,col,0] = xtalkLen[rng][twr,row,online_face,col,0]+1
                                length = xtalkLen[rng][twr,row,online_face,col,0]
                                xtalkDAC[rng][twr,row,online_face,col,length-1] = newDAC
                                xtalkADC[rng][twr,row,online_face,col,length-1] = newADC
                                newDAC += extrapPitch


                            # append final point @ dac = extrapDACTo
                            xtalkMAX = zachUtil.linear_extrap(3500, 4095, extrapDACTo,
                                                              xtalk3500, xtalk4095)

                            xtalkLen[rng][twr,row,online_face,col,0] = xtalkLen[rng][twr,row,online_face,col,0]+1
                            length = xtalkLen[rng][twr,row,online_face,col,0]
                            xtalkDAC[rng][twr,row,online_face,col,length-1] = extrapDACTo
                            xtalkADC[rng][twr,row,online_face,col,length-1] = xtalkMAX

    ## normalize xtalk ##
    log.info("Normalize xtalk")
    for rng in range(2,4):
        xtalkDAC[rng] = xtalkDAC[rng] * XTALK_FACTOR

    log.info("Re-Building xtalk splines")
    xtalkSplines = zachUtil.build_inl_splines(xtalkData, xtalkTwrSet)
    (adc2dacXtalk, dac2adcXtalk) = xtalkSplines

    ## output new xtalk file ##
    (basename,ext) = os.path.splitext(xtalkPath)
    xtalkOutPath = basename + ".xdiodeXtalkNormalized.xml"
    log.info("output final xtalk values:%s",xtalkOutPath)
    xtalkOutFile = calCalibXML.calIntNonlinCalibXML(xtalkOutPath, calCalibXML.MODE_CREATE)
    xtalkOutFile.write(xtalkLen, xtalkDAC, xtalkADC, tems=xtalkTwrSet)
    xtalkOutFile.close()
    calCalibXML.insertDTD(xtalkOutPath, dtdPath)

    ## add xtalk ##
    log.info("Applying xtalk correction")
    for twr in inTwrSet:
        for lyr in range(8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(12):
                for face in range(2):
                    online_face = calConstant.offline_face_to_online[face]
                    for rng in range(2,4):
                        for idx in range(inLen[rng][twr,row,online_face,col,0]):
                            # get dac value for this data point #
                            dac = inDAC[rng][twr,row,online_face,col,idx]
                            adc = inADC[rng][twr,row,online_face,col,idx]
                            spline =  dac2adcXtalk[(twr,row,online_face,col,rng)]
                                             
                            # add crosstalk to adc for corresponding data point #
                            xtalk_adc = spline.Eval(dac)
                            new_adc = adc + xtalk_adc

                            #print twr, lyr, col, face, rng, dac, adc, xtalk_adc, new_adc

                            inADC[rng][twr,row,online_face,col,idx] = new_adc

                        # check if we've saturated the adc scale
                        saturated = inADC[rng][twr,row,online_face,col] > .99*4095
                        try:
                            index = saturated.tolist().index(1)
                        except ValueError:
                            # skip top next channel if this one is not saturated.
                            continue

                        # set new length to included this last point
                        inLen[rng][twr,row,online_face,col,0] = index+1
                        # extrapolate this last point out to adc = 4095
                        adc1 = inADC[rng][twr,row,online_face,col,index-2]
                        adc2 = inADC[rng][twr,row,online_face,col,index-1]
                        dac1 = inDAC[rng][twr,row,online_face,col,index-2]
                        dac2 = inDAC[rng][twr,row,online_face,col,index-1]
                        adc3 = 4095
                        dac3 = zachUtil.linear_extrap(adc1,adc2,adc3,
                                                      dac1,dac2)
                        # clip dac to 4095 if it goes over
                        dac3 = min(dac3,4095)

                        # reset last point
                        inADC[rng][twr,row,online_face,col,index] = adc3
                        inDAC[rng][twr,row,online_face,col,index] = dac3

    ## write output file ##
    log.info("Building new inl file: %s"%outPath)
    outFile = calCalibXML.calIntNonlinCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(inLen,inDAC,inADC,tems=inTwrSet)
    outFile.close()

    calCalibXML.insertDTD(outPath, dtdPath)

    sys.exit(0)
