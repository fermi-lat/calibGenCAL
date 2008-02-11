"""
Tool to generate CAL offline tholdCI calibration XML file from space delimited TXT file.
Note: tholdCITXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).
Note: tholdCITXT2XML assumes 10 data points from xtal, one crystal width apart.

tholdCITXT2XML [-doptional.dtd] input.txt output.xml

where:
    -d           = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')

    input.txt    = tholdCI input txt file, space delimited in following format: (one line per xtal face)
                   "twr lyr col face lac fle fhe uld0 uld1 uld2 uld3 ped0 ped1 ped2 ped3"

        
    output.xml = properly formatted offline CAL tholdCI XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL TholdCI calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:49 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import numarray
import sets
import array

import calCalibXML
import calConstant
import cgc_util

#######################################################################################3

if __name__ == '__main__':

    #constants
    dtdName    = "calCalib_v2r3.dtd" #default value
 
    # setup logger
    logging.basicConfig()
    log = logging.getLogger('tholdCITXT2XML')
    log.setLevel(logging.INFO)


    # get environment settings
    try:
        calibGenCALRoot = os.environ['CALIBGENCALROOT']
    except:
        log.error('CALIBGENCALROOT must be defined')
        sys.exit(1)    

    # parse commandline
    #  - code stolen from: http://python.active-venture.com/lib/module-getopt.html
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.exception("getopt exception: " + __doc__)
        sys.exit(-1)

    for o, a in opts:
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibGenCALRoot, 'xml', dtdName)

    if (len(args) != 2):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)


    inPath = args[0]
    outPath = args[1]


    # create empty data arrays
    adcData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE, 3), numarray.Float32)
    uldData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE, 4), numarray.Float32)
    pedData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE, 4), numarray.Float32)

    # keep track of active towers
    twrSet = set()

    # read input files
    inFile = open(inPath, 'r')
    lines = inFile.readlines()

    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue

        # read in values from current line
        [twr, lyr, col, face, 
         lac, fle, fhe,
         uld0, uld1, uld2, uld3,
         ped0, ped1, ped2, ped3] = line.split()

        # convert array index values to integer.
        twr = int(twr)
        lyr = int(lyr)
        col = int(col)
        face = int(face)

        # make sure current tower is on list
        twrSet.add(twr)

        # get online row indexing (as opposed to offline lyr indexing)
        row = calCalibXML.layerToRow(int(lyr))
        # convert offline face numbering to online face numbering
        online_face = calConstant.offline_face_to_online[face]

        # populate arrays
        adcData[twr,row,online_face,col,0] = float(lac)
        adcData[twr,row,online_face,col,1] = float(fle)
        adcData[twr,row,online_face,col,2] = float(fhe)

        uldData[twr,row,online_face,col,0] = float(uld0)
        uldData[twr,row,online_face,col,1] = float(uld1)
        uldData[twr,row,online_face,col,2] = float(uld2)
        uldData[twr,row,online_face,col,3] = float(uld3)

        pedData[twr,row,online_face,col,0] = float(ped0)
        pedData[twr,row,online_face,col,1] = float(ped1)
        pedData[twr,row,online_face,col,2] = float(ped2)
        pedData[twr,row,online_face,col,3] = float(ped3)


    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calTholdCICalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(adcData, uldData, pedData, tems = twrSet)
    outFile.close()

    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)
    sys.exit(0)                            
