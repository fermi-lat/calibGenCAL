"""
Tool to generate CAL offline MeVPerDAC calibration XML file from space delimited TXT file.
Note: mpdTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).


mpdTXT2XML [-doptional.dtd] input.txt output.xml

where:
       -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r2.dtd')
       input.txt  = input txt file, space delimited in following format:
                      twr lyr col diode mevPerDAC sigma
       output.xml = properly formatted offline CAL mevPerDAC XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL mevPerDAC calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2005/09/15 18:15:08 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import sets
import array

import calCalibXML
import calConstant

#######################################################################################

if __name__ == '__main__':

    # constants
    usage      = "mpdTXT2XML [-doptional.dtd] input.txt output.xml"
    dtdName    = "calCalib_v2r2.dtd"
    nTXTFields = 6
    DIODE_LRG  = 0
    DIODE_SM   = 1

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('mpdTXT2XML')
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

    for o, a in opts:
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

    if (len(args) != 2):
        log.error("Need 2 filenames: " + usage)
        sys.exit(1)
        
    inPath = args[0]
    outPath = args[1]

    # read input file
    inFile = open(inPath, 'r')

    lines = inFile.readlines()

    outData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE, 8), Numeric.Float32)

    # indeces for outData
    bigValIdx   = 0
    smallValIdx = 1
    bigSigIdx   = 2
    smallSigIdx = 3

    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1
        (twr, lyr, col, diode, mpd, sig)= line.split()
        # convert array index values to integer.
        twr = int(twr)
        lyr = int(lyr)
        col = int(col)
        diode = int(diode)
        mpd = float(mpd)
        sig = float(sig)

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))

        valIdx = bigValIdx
        sigIdx = bigSigIdx
        if (diode == DIODE_SM):
            valIdx = smallValIdx
            sigIdx = smallSigIdx
            
        outData[twr, row, col, valIdx] = mpd
        outData[twr, row, col, sigIdx] = sig

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calMevPerDacCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)


    sys.exit(0)                            

    
