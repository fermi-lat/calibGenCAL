"""
Tool to apply calibGain correction to asymmetry from calibGain=ON to calibGain=OFF

The commandline is:
asymApplyCalibGain [-V] [-doptional.dtd] <calibGainCoef.txt> <input_asym.xml> <output_asym.xml>


where:
    -d                   = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
    -V                   = verbose; turn on debug output
    <calibGainCoef.txt>  = output from calibGainCoef.py script
    <input_asym.xml>      = input offline asymmetry xml calibration file
    <output_asym.xml>     = output asymmetry file.
"""

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to asymmetry xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/06/29 16:02:41 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import ConfigParser
import getopt
import Numeric
import calCalibXML
import calConstant
import calibGain
import zachUtil

if __name__ == '__main__':
    usage = "asymApplyCalibGain [-V] [-doptional.dtd] <calibGainCoef.txt> <input_asym.xml> <output_asym.xml>"
    dtdName    = "calCalib_v2r3.dtd" #default value

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('asymApplyCalibGain')
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
    

    # open and read XML Asymmetry file
    log.info("Reading asym XML file: " + inPath)
    inFile = calCalibXML.calAsymCalibXML(inPath)
    (xposData, asymData) = inFile.read()
    towers = inFile.getTowers()
    inFile.close()

    
    # open and read calibGainRatio txt file
    log.info("Reading calibGain TXT file: " +  calibGainPath)
    (calibGainRatio, twrSet) = calibGain.read_calibGain_txt(calibGainPath)

    for twr in twrSet:
        # use online face numbering
        calibGainNFace = Numeric.log(calibGainRatio[twr,:,0,:])
        calibGainPFace = Numeric.log(calibGainRatio[twr,:,1,:])

        # both arrays use same indexing scheme.
        # adjust HE value only
        # calibGainRatio is dacOff/dacOn
        # muon asymmetry is taken w/ calibGain off, we want to convert to cgOn

        # ASYM_LS
        # muon_asym_LS = log(dacPosLarge/dacNegSmallCGOff)
        # cgNFace      = log(dacNegSmallCGOff/dacNegSmallCGOn)
        # muon_asym_LS + cgNFace = log(dacPosLage/dacNegSmallCgOn) = flight_asym_LS
        asymLS = asymData[twr,:,:,zachUtil.asymIdx[(0,1,False)],:]
        asymLS += calibGainNFace[...,Numeric.NewAxis]
        

        # ASYM_SL
        # muon_asym_SL = log(dacPosSmallCGOff/dacNegLarge)
        # cgPFace = log(dacPosSmallCGOff/dacPosSmallCGOn)
        # muon_asym_SL - cgPFace = log(dacPosSmallCGOn/dacLarge) = flight_asym_SL
        # val
        asymSL = asymData[twr,:,:,zachUtil.asymIdx[(1,0,False)]]
        asymSL -= calibGainPFace[...,Numeric.NewAxis]

        # ASYM_SS
        # muon_asym_SS = log(dacPosSmallCGOff/dacNegSmallCGOff)
        # cgNFace      = log(dacNegSmallCGOff/dacNegSmallCGOn)
        # cgPFace      = log(dacPosSmallCGOff/dacPosSmallCGOn)
        # muon_asym_ss - cgPFace + cgNFace = log(dacPosSmallCGOn/dacNegSmallCGOn)
        asymSS = asymData[twr,:,:,zachUtil.asymIdx[(1,1,False)]]
        asymSS += calibGainNFace[...,Numeric.NewAxis]
        asymSS -= calibGainPFace[...,Numeric.NewAxis]

    log.info("Writing asym XML file: " + outPath)
    outFile = calCalibXML.calAsymCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(xposData, asymData, tems = twrSet)
    outFile.close()

    calCalibXML.insertDTD(outPath, dtdPath)

    sys.exit(0)
    
