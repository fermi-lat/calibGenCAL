"""
Tool to merge mutilple single tower CAL Ped calibration XML files into a
single output file.  The command line is:

pedMerge [-V] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The merged CAL Ped calibration XML file to output.
"""


__facility__  = "Offline"
__abstract__  = "Tool to merge mutilple CAL Ped calibration XML files."
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/09/06 13:28:12 $"
__version__   = "$Revision: 1.8 $, $Author: dwood $"
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
    Represents one pedMerge input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name, pedData):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        Param: pedData A Numeric ADC data arrays from the input file.
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name
        self.pedData = pedData


#######################################################################################3


if __name__ == '__main__':

    usage = "pedMerge [-V] <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('pedMerge')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)        
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    outName = args[1]

    # get environment settings

    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    


    # read config file settings

    log.info("Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("Config file %s missing or empty" % configName)
        sys.exit(1)

    # get input file names and tower ID's

    if 'infiles' not in sections:
        log.error("Config file %s missing [infiles] section" % configName)
        sys.exit(1)

    if 'dtdfiles' not in sections:
        log.error("Config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)    

    inFiles = []
    options = configFile.options('infiles')
    for opt in options:
        optList = opt.split('_')
        if len(optList) != 2 or optList[0] != 'ped':
            continue
        destTwr = int(optList[1])
        if destTwr < 0 or destTwr > 15:
            log.error("Dest index for [infiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
        value = configFile.get('infiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 1:
            name = nameList[0]
            srcTwr = 0
        elif nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [infiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
        inFile = inputFile(srcTwr, destTwr, name, None)
        inFiles.append(inFile)
        log.debug('Adding file %s to input as tower %d', name, destTwr)


    # get DTD file name

    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("Config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)
    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))


    # read input files

    firstFile = True
    for f in inFiles:
        log.info('Reading file %s', f.name)
        inFile = calCalibXML.calPedCalibXML(f.name)
        pedData = inFile.read()
        f.pedData = pedData
        if firstFile:
            info = inFile.info()
            firstFile = False
        inFile.close()

    log.debug('Using ouput info:\n%s', str(info))

    outData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE,
                             calConstant.NUM_RNG, 3), Numeric.Float32)

    for f in inFiles:
        pedData = f.pedData
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(calConstant.NUM_RNG):
                        for val in range(3):
                            x = pedData[f.srcTwr, row, end, fe, erng, val]
                            outData[f.destTwr, row, end, fe, erng, val] = x

    log.info('Writing output file %s', outName)

    temList = []
    for f in inFiles:
        temList.append(f.destTwr)

    outFile = calCalibXML.calPedCalibXML(outName, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = temList)
    outFile.close()


    # fixup calibration XML file - insert DTD info

    calCalibXML.insertDTD(outName, dtdName)

    sys.exit(0)                            