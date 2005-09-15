"""
Tool to generate CAL offline asymmetry calibration XML file from space delimited TXT file.
Note: asymTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).
Note: asymTXT2XML assumes 10 data points from xtal, one crystal width apart.

asymTXT2XML [-doptional.dtd] input.txt output.xml

where:
    -d           = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r2.dtd')

    input.txt    = asymmetry input txt file, space delimited in following format:
                   twr lyr col diode_mnem asym0 sig0 asym1 sig1 ... asym9 sig9

                   where diode_mnem = 'LL'|'LS'|'SL'|'SS'
        
    output.xml = properly formatted offline CAL asymmetry XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL Asymmetry calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2005/09/12 17:44:28 $"
__version__   = "$Revision: 1.10 $, $Author: dwood $"
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
    nTXTFields = 24

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
    # indeces for outData
    valIdx = {'LL':0, 'SS':1, 'LS':2, 'SL':3}
    sigIdx = {'LL':4, 'SS':5, 'LS':6, 'SL':7}

    # keep track of active towers
    twrSet = set()

    # read input files
    inFile = open(inPath, 'r')
    lines = inFile.readlines()

    nLine = -1
    for line in lines:
        nLine+=1
        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col diode_mnem asym0 sig0 asym1 sig1 ... asym9 sig9]")
            sys.exit(-1)

        # convert array index values to integer.
        twr = int(vals[0])
        lyr = int(vals[1])
        col = int(vals[2])
        diode_set = vals[3]

        data_pts = vals[4:]
        # convert vals array to floats instead of stringns
        for i in range(len(data_pts)):
            data_pts[i] = float(data_pts[i])

        # make sure current tower is on list
        twrSet.add(twr)

        row = calCalibXML.layerToRow(int(lyr))

        # set asym & sigma for each point alon xtal
        for i in range(len(xpos_pts)):
            outData[twr, row, col, valIdx[diode_set],i] = data_pts[i*2]
            outData[twr, row, col, sigIdx[diode_set],i] = data_pts[i*2+1]

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calAsymCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(xpos_pts, outData, tems = twrSet)
    outFile.close()


    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)

    sys.exit(0)                            
                      

