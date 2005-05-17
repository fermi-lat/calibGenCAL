"""
Tool to produce CAL TholdCI XML calibration data files.  The command line is:

tholdCIGen [-V] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The merged CAL TholdCI calibration XML file to output.
"""

import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calDacXML
import calFitsXML
import calCalibXML



class inputFile:
    """
    Represents one tho input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        Param: adcData A Numeric ADC data array from the input file.
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name


##################################################################################        


if __name__ == '__main__':


    usage = "tholdCIGen [-V] <cfg_file> <out_xml_file>"


    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)

    # get environment settings

    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('genTholdCI: CALIBUTILROOT must be defined')
        sys.exit(1)    

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
    calibName = args[1]


    # read config file settings

    log.info("genTholdCI: Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("genTholdCI: config file %s missing or empty" % configName)
        sys.exit(1)

    # get DAC setttings file names

    if 'dacfiles' not in sections:
        log.error("genTholdCI: config file %s missing [dacfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dacfiles', 'snapshot'):
        log.error("genTholdCI: config file %s missing [dacfiles]:snapshot option" % configName)
        sys.exit(1)

    snapshotName = configFile.get('dacfiles', 'snapshot')


    # get DAC/ADC characterization file names

    if 'adcfiles' not in sections:
        log.error("genTholdCI: config file %s missing [adcfiles] section" % configName)
        sys.exit(1)

    uldFiles = []
    lacFiles = []
    fleFiles = []
    fheFiles = []
    pedFiles = []

    options = configFile.options('adcfiles')
    for opt in options:
        
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        
        if optList[0] == 'uld2adc':
            fList = uldFiles
        elif optList[0] == 'lac2adc':
            fList = lacFiles
        elif optList[0] == 'fle2adc':
            fList = fleFiles
        elif optList[0] == 'fhe2adc':
            fList = fheFiles
        elif optList[0] == 'pedestals':
            fList = pedFiles
        else:
            continue

        destTwr = int(optList[1])        
        if destTwr < 0 or destTwr > 15:
            log.error("genTholdCI: index for [adcfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
            
        value = configFile.get('adcfiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("genTholdCI: incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("genTholdCI: src index for [infiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)    
        inFile = inputFile(srcTwr, destTwr, name)
        fList.append(inFile)
        
        log.debug('genTholdCI: adding %s file %s to input as tower %d (from %d)', optList[0], name,
                  destTwr, srcTwr)

    if not configFile.has_option('adcfiles', 'intnonlin'):
        log.error("config file %s missing [adcfiles]:intnonlin option" % configName)
        sys.exit(1)
    intNonlinName = configFile.get('adcfiles', 'intnonlin')


    # get DTD spec file names

    if 'dtdfiles' not in sections:
        log.error("config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)

    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))


    # read snapshot DAC config file

    log.info("genTholdCI: Reading file %s", snapshotName)
    snapshotFile = calDacXML.calSnapshotXML(snapshotName)
    fleDacData = snapshotFile.read('fle_dac')
    fheDacData = snapshotFile.read('fhe_dac')
    lacDacData = snapshotFile.read('log_acpt')
    uldDacData = snapshotFile.read('rng_uld_dac')
    config0Data = snapshotFile.read('config_0')
    tlist =  snapshotFile.getTowers()
    snapshotFile.close()

    # get gain indicies

    leGainData = (config0Data & 0x0007)
    heGainData = ((config0Data & 0x0078) >> 3)

    # create empty ADC data arrays

    pedData = Numeric.zeros((16, 9, 4, 8, 2, 12), Numeric.Float32)
    uldAdcData = Numeric.zeros((3, 16, 8, 2, 12, 128), Numeric.Float32)
    lacAdcData = Numeric.zeros((16, 8, 2, 12, 128), Numeric.Float32)
    fleAdcData = Numeric.zeros((16, 8, 2, 12, 128), Numeric.Float32)
    fheAdcData = Numeric.zeros((16, 8, 2, 12, 128), Numeric.Float32)

    # read LAC/ADC characterization file
    
    for f in lacFiles:    

        log.info("genTholdCI: Reading file %s", f.name)
        lacAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        adcData = lacAdcFile.read()
        lacAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        lacAdcFile.close()

    # read ULD/ADC characterization files

    for f in uldFiles:        
    
        log.info("genTholdCI: Reading file %s", f.name)
        uldAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        adcData = uldAdcFile.read()
        uldAdcData[:,f.destTwr,...] = adcData[:,f.srcTwr,...]
        uldAdcFile.close()

        
    # read FLE/ADC characterization files

    for f in fleFiles:
        
        log.info("genTholdCI: Reading file %s", f.name)
        fleAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        adcData = fleAdcFile.read()
        fleAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fleAdcFile.close()

    # read FHE/ADC characterization files

    for f in fheFiles:    

        log.info("genTholdCI: Reading file %s", f.name)
        fheAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        adcData = fheAdcFile.read()
        fheAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fheAdcFile.close()

    # read pedestal values file

    for f in pedFiles:    

        log.info("genTholdCI: Reading file %s", f.name)
        pedFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        pedData[f.destTwr,...] = pedFile.read()
        pedFile.close()        

    # read ADC non-linearity characterization data

    log.info("genTholdCI: Reading file %s", intNonlinName)
    intNonlinFile = calCalibXML.calIntNonlinCalibXML(intNonlinName)
    (intNonlinDacData, intNonlinAdcData) = intNonlinFile.read()
    intNonlinFile.close()

    # create CAL calibration output XML file

    log.info("genTholdCI: Creating file %s", calibName)
    calibFile = calCalibXML.calTholdCICalibXML(calibName, mode = calCalibXML.MODE_CREATE)
    dacData = (uldDacData, lacDacData, fleDacData, fheDacData)
    adcData = (uldAdcData, lacAdcData, fleAdcData, fheAdcData)
    calibFile.write(dacData, adcData, intNonlinAdcData[3], pedData, leGainData, heGainData,
                    tems = tlist)
    calibFile.close()

    # make copy of XML output file

    calibFile = open(calibName, 'r')
    lines = calibFile.readlines()
    calibFile.close()

    # fixup calibration XML file - insert DTD info

    dtdStr = '<!DOCTYPE calCalib ['
    dtdFile = open(dtdName, 'r')
    dtdLines = dtdFile.readlines()
    dtdFile.close()
    for l in dtdLines:
        dtdStr += l
    dtdStr += ']>\n'

    calibFile = open(calibName, 'w')
    calibFile.write(lines[0])
    calibFile.write(dtdStr)
    calibFile.writelines(lines[1:])
    calibFile.close()

    calibFile = calCalibXML.calTholdCICalibXML(calibName)
    info = calibFile.info()
    (adcData, uldData, pedData) = calibFile.read()
    calibFile.close()

    sys.exit(0)



