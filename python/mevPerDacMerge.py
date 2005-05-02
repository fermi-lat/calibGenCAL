"""
Tool to merge mutilple single tower CAL MevPerDac calibration XML files into a
single output file.  The command line is:

mevPerDacMerge <cfg_file> <out_xml_file>

where:
    <cfg_file> = The application configuration file to use.
    <out_xml_file> = The merged CAL MevPerDac calibration XML file to output.
"""


__facility__  = "Offline"
__abstract__  = "Tool to merge mutilple CAL MevPerDac calibration XML files."
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/14 19:17:17 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
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
    Represents one mevPerDacMerge input XML file.
    """
    
    def __init__(self, twr, name, energyData):
        """
        inputFile constructor

        Param: twr The tower number (0 - 15)
        Param: name The input file name
        Param: pedData A Numeric ADC data arrays from the input file.
        """
        
        self.twr = twr
        self.name = name
        self.energyData = energyData


#######################################################################################3


if __name__ == '__main__':

    usage = "mevPerDacMerge <cfg_file> <out_xml_file>"

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


    # get environment settings

    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('mevPerDacMerge: CALIBUTILROOT must be defined')
        sys.exit(1)

    # read config file settings

    log.info("mevPerDacMerge: Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("mevPerDacMerge: config file %s missing or empty" % configName)
        sys.exit(1)

    # get input file names and tower ID's

    if 'infiles' not in sections:
        log.error("pedMerge: config file %s missing [infiles] section" % configName)
        sys.exit(1)

    if 'dtdfiles' not in sections:
        log.error("mevPerDacMerge: config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)    

    inFiles = []
    options = configFile.options('infiles')
    for opt in options:
        optList = opt.split('_')
        if len(optList) != 2 or optList[0] != 'mevperdac':
            log.error("mevPerDacMerge: unknown option %s in section [infiles]", opt)
            sys.exit(1)
        twr = int(optList[1])
        if twr < 0 or twr > 15:
            log.error("mevPerDacMerge: index for [infiles] option %s out of range (0 - 15)", opt)
        name = configFile.get('infiles', opt)
        inFile = inputFile(twr, name, None)
        inFiles.append(inFile)
        log.debug('mevPerDacMerge: adding file %s to input as tower %d', name, twr)


    # get DTD file name

    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("mevPerDacMerge: config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)
    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))


    # read input files

    firstFile = True
    for f in inFiles:
        log.info('mevPerDacMerge: reading file %s', f.name)
        inFile = calCalibXML.calMevPerDacCalibXML(f.name)
        energyData = inFile.read()
        f.energyData = energyData
        if firstFile:
            info = inFile.info()
            firstFile = False
        inFile.close()

    log.debug('mevPerDacMerge: using ouput info:\n%s', str(info))

    outData = Numeric.zeros((16, 8, 12, 8), Numeric.Float32)

    for f in inFiles:
        energyData = f.energyData
        for row in range(8):
            for fe in range(12):
                for val in range(8):
                            x = energyData[0, row, fe, val]
                            outData[f.twr, row, fe, val] = x

    log.info('mevPerDacMerge: writing output file %s', outName)

    temList = []
    for f in inFiles:
        temList.append(f.twr)

    outFile = calCalibXML.calMevPerDacCalibXML(outName, calCalibXML.MODE_CREATE)
    outFile.write(outData, tems = temList)
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
    