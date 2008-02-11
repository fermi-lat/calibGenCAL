"""
Tool to generate CAL offline pedestal calibration XML file from space delimited TXT file.
Note: pedTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).

pedTXT2XML [-doptional.dtd] input.txt output.xml

where:
       -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
       input.txt  = input txt file, space delimited in following format:
                      twr lyr col face erange pedestal sigma
       output.xml = properly formatted offline CAL pedestal XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL Ped calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:50 $"
__version__   = "$Revision: 1.10 $, $Author: fewtrell $"
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

#######################################################################################

if __name__ == '__main__':

    # constants
    dtdName    = "calCalib_v2r3.dtd" #default value
    nTXTFields = 7

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('pedTXT2XML')
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

    for o, a in opts:
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

    if (len(args) != 2):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)
        
    inPath = args[0]
    outPath = args[1]

    # read input file
    inFile = open(inPath, 'r')

    lines = inFile.readlines()

    # define output array
    outData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE,
                             calConstant.NUM_RNG, 3), numarray.Float32)
    valIdx = 0 # index into outData last field
    sigIdx = 1 # index into outData last field


    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1

        if line[0] == ';':
            continue

        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face erange pedestal sigma]")
            sys.exit(-1)

        # convert vals array to floats instead of strings
        vals = [float(x) for x in vals]

        (twr, lyr, col, face, rng, ped, sig) = vals
        
        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)
        rng  = int(rng)

        # convert offline face numbering to online face numbering
        face = calConstant.offline_face_to_online[face]

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))
        outData[twr, row, face, col, rng, valIdx] = ped
        outData[twr, row, face, col, rng, sigIdx] = sig

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calPedCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)


    sys.exit(0)                            

    
