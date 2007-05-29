"""
Tool to generate CAL bias XML file from space delimited TXT file 
Note: biasTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).

biasTXT2XML input.txt output.xml

where:
       input.txt  = input txt file, space delimited in following format:
                      twr lyr col face muThresh err muThreshWidth err ciThresh err ciThreshWidth err muThresh/ciThresh delPed
       output.xml = properly formatted CAL bias XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL BIAS calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/08/03 13:11:03 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name: v4r4 $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import sets
import array

import calDacXML
import calCalibXML
import calConstant
import zachUtil

#######################################################################################

if __name__ == '__main__':

    # constants
    usage      = "biasTXT2XML input.txt output.xml"
    nTXTFields = 14

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('biasTXT2XML')
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
                             2),
                            Numeric.Float32)

    # keep track of active towers
    twrSet = set()

    nLine = -1
    for line in lines:
        nLine+=1
        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face muThresh err muThreshWidth err ciThresh err ciThreshWidth err muThresh/ciThresh delPed]")
            sys.exit(-1)

        # convert vals array to floats instead of strings
        vals = [float(x) for x in vals]

        (twr, lyr, col, face, muThresh, err, muThreshWidth, err, ciThresh, err, ciThreshWidth, err, ratio, delPed) = vals
         
        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)

        # convert offline face numbering to online face numbering
        face = zachUtil.offline_face_to_online[face]

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))

        bias = muThresh - ciThresh

        # output fle value for both fle and fhe
        outData[twr, row, face, col, 0] = bias
        outData[twr, row, face, col, 1] = bias
        

    log.info('Writing output file %s', outPath)

    outFile = calDacXML.calEnergyXML(outPath, 'thrBias', calDacXML.MODE_CREATE)
    outFile.write(outData, tems = twrSet)
    outFile.close()

    sys.exit(0)                            

    
