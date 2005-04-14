"""
Tool to merge mutilple single tower CAL IntNonlin calibration XML files into a
single output file.
"""


__facility__  = "Offline"
__abstract__  = "Tool to merge mutilple CAL IntNonlin calibration XML files."
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/14 19:07:44 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calCalibXML
import calConstant



class inputFile:
    """
    Represents one intNonlinMerge input XML file.
    """
    
    def __init__(self, twr, name, dacData, adcData):
        """
        inputFile constructor

        Param: twr The tower number (0 - 15)
        Param: name The input file name
        Param: dacData A list of Numeric DAC data arrays from the input file.
        Param: dacData A list of Numeric ADC data arrays from the input file.
        """
        
        self.twr = twr
        self.name = name
        self.dacData = dacData
        self.adcData = adcData



#######################################################################################3


if __name__ == '__main__':

    usage = "intNonlinMerge <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    outName = args[1]


    # read config file settings

    log.info("intNonlinMerge: Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("intNonlinMerge: config file %s missing or empty" % configName)
        sys.exit(1)

    # get input file names and tower ID's

    if 'infiles' not in sections:
        log.error("intNonlinMerge: config file %s missing [infiles] section" % configName)
        sys.exit(1)

    if 'dtdfiles' not in sections:
        log.error("intNonlinMerge: config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)    

    inFiles = []
    options = configFile.options('infiles')
    for opt in options:
        optList = opt.split('_')
        if len(optList) != 2 or optList[0] != 'intnonlin':
            log.error("intNonlinMerge: unknown option %s in section [infiles]", opt)
            sys.exit(1)
        twr = int(optList[1])
        if twr < 0 or twr > 15:
            log.error("intNonlinMerge: index for [infiles] option %s out of range (0 - 15)", opt)
        name = configFile.get('infiles', opt)
        inFile = inputFile(twr, name, None, None)
        inFiles.append(inFile)
        log.debug('intNonLinMerge: adding file %s to input as tower %d', name, twr)


    # get DTD file name

    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("intNonlinMerge: config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)
    dtdName = configFile.get('dtdfiles', 'dtdfile')


    # read input files

    firstFile = True
    for f in inFiles:
        log.info('intNonlinMerge: reading file %s', f.name)
        inFile = calCalibXML.calIntNonlinCalibXML(f.name)
        (dacData, adcData) = inFile.read()
        f.adcData = adcData
        f.dacData = dacData
        if firstFile:
            info = inFile.info()
            firstFile = False
        inFile.close()

    log.debug('intNonlinMerge: using ouput info:\n%s', str(info))


    # merge tower DAC data

    dacDataOut = [None, None, None, None]
    dacDataLen = [None, None, None, None]
    
    for erng in range(4):
        maxX = 0
        for f in inFiles:
            s = len(f.dacData[erng])
            if s > maxX:
                maxX = s
                dacDataOut[erng] = f.dacData[erng]
                dacDataLen[erng] = s
                log.debug('intNonlinMerge: using ouput DAC values for range %s:\n%s', calConstant.CRNG[erng], \
                          dacDataOut[erng])
            

    # create empty output ADC data array

    adcDataOut = [None, None, None, None]
    for erng in range(4):
        adcDataOut[erng] = Numeric.zeros((16, 8, 2, 12, dacDataLen[erng]), Numeric.Float32)
        log.debug('intNonlinMerge: using output ADC array shape %s for range %s', str(adcDataOut[erng].shape),
                  calConstant.CRNG[erng])
                

    # merge tower ADC data

    for erng in range(4):
        for f in inFiles:
            adcData = f.adcData
            inData = adcData[erng]
            outData = adcDataOut[erng]
            for row in range(8):
                for end in range(2):
                    for fe in range(12):
                        for dac in range(inData.shape[-1]):
                            x = inData[0, row, end, fe, dac]
                            outData[f.twr, row, end, fe, dac] = x
            

    log.info('intNonlinMerge: writing output file %s', outName)

    temList = []
    for f in inFiles:
        temList.append(f.twr)

    outFile = calCalibXML.calIntNonlinCalibXML(outName, calCalibXML.MODE_CREATE)
    outFile.write(dacDataOut, adcDataOut, startTime = info['startTime'], stopTime = info['stopTime'], \
                  triggers = info['triggers'], mode = info['mode'], source = info['mode'], \
                  tems = temList)
    outFile.close()


    # fixup calibration XML file - insert DTD info

    outFile = open(outName, 'r')
    lines = outFile.readlines()
    outFile.close()

    dtdStr = '<!DOCTYPE calCalib ['
    dtdFile = open(dtdName, 'r')
    dtdLines = dtdFile.readlines()
    dtdFile.close()
    for l in dtdLines:
        dtdStr += l
    dtdStr += ']>\n'

    outFile = open(outName, 'w')
    outFile.write(lines[0])
    outFile.write(dtdStr)
    outFile.writelines(lines[1:])
    outFile.close()

    sys.exit(0)

