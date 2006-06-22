"""
Tool to generate CAL offline asymmetry calibration XML file from space delimited TXT file.
Note: asymTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).
Note: asymTXT2XML assumes 10 data points from xtal, one crystal width apart.

asymTXT2XML [-doptional.dtd] input.txt output.xml

where:
    -d           = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r2.dtd')

    input.txt    = asymmetry input txt file, space delimited in following format:
                   twr lyr col pos_diode neg_diode xpos asym asig
        
    output.xml = properly formatted offline CAL asymmetry XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL Asymmetry calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/02/22 21:20:06 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
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

#######################################################################################3

if __name__ == '__main__':

    #constants
    usage      = "asymTXT2XML [-doptional.dtd] input.txt output.xml"
    dtdName    = "calCalib_v2r2.dtd" #default value
    xpos_pts   = (-122.25, -95.0833, -67.9167, -40.75, -13.5833, 13.5833, 40.75, 67.9167, 95.0833, 122.25)

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('asymTXT2XML')
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
        log.exception("getopt exception: " + usage)
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


    outData = Numeric.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_FE,
                             8,
                             len(xpos_pts)),
                            Numeric.Float32)
    # keep track of active towers
    twrSet = set()

    # read input files
    inFile = open(inPath, 'r')
    lines = inFile.readlines()

    # associate pos_diode, big_diode & val_or_sigma
    # to index into calAsymCalibXML array as spec'd as
    # follows (from calCalibXML.py)

    #     The next-to-last
    #     dimension contains the following data:
    #     0 = bigVals value
    #     1 = smallVals value
    #     2 = NsmallPbigVals value
    #     3 = PsmallNbigVals value
    #     4 = bigSigs value
    #     5 = smallSigs value
    #     6 = NsmallPbigSigs value
    #     7 = PsmallNbigSigs value

    # tuple idx keys are (pos_diode, neg_diode, sigma)
    asymIdx = {
        (0,0,False):0,\
        (1,1,False):1,\
        (0,1,False):2,\
        (1,0,False):3,\
        (0,0,True):4,\
        (1,1,True):5,\
        (0,1,True):6,\
        (1,0,True):7}

    # keep track of which curve we're plotting
    # use tuple as key of (twr,lyr,col,pdiode,ndiode)
    # increment value for each new point.
    curveDict = {}
    

    nLine = -1
    for line in lines:
        nLine+=1
        [twr,lyr,col,pdiode,ndiode,asym,sig] = line.split()

        # convert array index values to integer.
        twr = int(twr)
        lyr = int(lyr)
        col = int(col)
        pdiode = int(pdiode)
        ndiode = int(ndiode)
        asym   = float(asym)
        sig    = float(sig)

        curveId = (twr,lyr,col,pdiode,ndiode)

        # keep track of which curve we're on
        if curveDict.has_key(curveId):
            curveDict[curveId] += 1
        else:
            curveDict[curveId] = 0

        nPt = curveDict[curveId]

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))

        # calculate index for asym_type
        valIdx = asymIdx[(pdiode,ndiode,False)]
        sigIdx = asymIdx[(pdiode,ndiode,True)]

        # set asym & sigma for each point alon xtal
        outData[twr, row, col, valIdx, nPt] = asym
        outData[twr, row, col, sigIdx, nPt] = sig

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calAsymCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(xpos_pts, outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)

    sys.exit(0)                            
                      

