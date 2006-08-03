"""
Tool to generate CAL offline MuSlope calibration XML file from space delimited TXT file.
Note: muSlopeTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).

muSlopeTXT2XML [-doptional.dtd] input.txt output.xml

where:
       -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
       input.txt  = input txt file, space delimited in following format:
                      twr lyr col face erange muSlope error
       output.xml = properly formatted offline CAL MuSlope XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL MUSLOPE calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/06/22 21:21:20 $"
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
import zachUtil

#######################################################################################

if __name__ == '__main__':

    # constants
    usage      = "muSlopeTXT2XML [-doptional.dtd] input.txt output.xml"
    dtdName    = "calCalib_v2r3.dtd" #default value
    nTXTFields = 7

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('muSlopeTXT2XML')
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

    # define output array
    outData = Numeric.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_END,
                             calConstant.NUM_FE,
                             calConstant.NUM_RNG,
                             2),
                            Numeric.Float32)
    slopeIdx = 0 # index into outData last field
    errIdx = 1 # index into outData last field


    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1
        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face erange muSlope error]")
            sys.exit(-1)

        # convert vals array to floats instead of strings
        for i in range(len(vals)):
            vals[i] = float(vals[i])

        (twr, lyr, col, face, rng, muSlope, err) = vals
        
        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)
        rng  = int(rng)

        # convert offline face numbering to online face numbering
        face = zachUtil.offline_face_to_online[face]

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))
        outData[twr, row, face, col, rng, slopeIdx] = muSlope
        outData[twr, row, face, col, rng, errIdx] = err

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calMuSlopeCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)


    sys.exit(0)                            

    
