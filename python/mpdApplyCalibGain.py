"""
Tool to apply calibGain correction to mevPerDAC from calibGain=ON to calibGain=OFF

The commandline is:
mpdApplyCalibGain [-V] [-doptional.dtd] <calibGainCoef.txt> <input_mpd.xml> <output_mpd.xml>


where:
    -d                   = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
    -V                   = verbose; turn on debug output
    <calibGainCoef.txt>  = output from calibGainCoef.py script
    <input_mpd.xml>      = input offline mevPerDAC xml calibration file
    <output_mpd.xml>     = output mevPerDAC file.
"""

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to mevPerDAC xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/08/11 16:30:18 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
__release__   = "$Name: v4r4 $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import ConfigParser
import getopt
import Numeric
import calCalibXML
import calConstant
import zachUtil

### CONSTANTS ###

if __name__ == '__main__':
    usage = "mpdApplyCalibGain [-V] [-doptional.dtd] <calibGainCoef.txt> <input_mpd.xml> <output_mpd.xml>"
    dtdName    = "calCalib_v2r3.dtd" #default value

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('mpdApplyCalibGain')
    log.setLevel(logging.INFO)

    # get environment settings
    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    

    # check command line
    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o,a in optList:
        if o == '-V':
            log.setLevel(logging.DEBUG)
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

    args = opts[1]
    if len(args) != 3:
        log.error(usage)
        sys.exit(1)

    calibGainPath = args[0]
    inPath = args[1]
    outPath = args[2]
    

    # open and read XML MevPerDAC file
    log.info("Reading mpd XML file: " + inPath)
    inFile = calCalibXML.calMevPerDacCalibXML(inPath)
    mpdData = inFile.read()
    towers = inFile.getTowers()
    inFile.close()

    
    # open and read calibGainRatio txt file
    log.info("Reading calibGain TXT file: " +  calibGainPath)
    (calibGainRatio, twrSet) = zachUtil.read_perFace_txt(calibGainPath)

    mean_ratio = Numeric.sqrt(calibGainRatio[:,:,0,:]*calibGainRatio[:,:,1,:])

    for twr in twrSet:
        # both arrays use same indexing scheme.
        # adjust HE value only
        # calibGainRatio is dacOff/dacOn
        # muon mevPerDAC is taken w/ calibGain off, we want to convert to cgOn
        # mev/dacOn = (mev/dacOff) * (dacOff/dacOn)
        mpdData[twr,...,1] *= mean_ratio[twr]

        
        # adjust error
        mpdData[twr,...,3] *= mean_ratio[twr]

    

    log.info("Writing mpd XML file: " + outPath)
    outFile = calCalibXML.calMevPerDacCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(mpdData, tems = twrSet)
    outFile.close()

    calCalibXML.insertDTD(outPath, dtdPath)

    sys.exit(0)
    
