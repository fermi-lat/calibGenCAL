"""
Tool to produce CAL TholdCI XML calibration data files.  The command line is:

tholdCIGen [-V] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The merged CAL TholdCI calibration XML file to output.
"""


__facility__  = "Offline"
__abstract__  = "Tool to produce CAL TholdCI XML calibration data files"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/03/14 21:18:05 $"
__version__   = "$Revision: 1.21 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calDacXML
import calFitsXML
import calCalibXML
import calConstant




class inputFile:
    """
    Represents one tho input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name, version = None):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        Param: adcData A Numeric ADC data array from the input file.
        Param: version The XML format version of the input file.
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name
        self.version = version


##################################################################################        


if __name__ == '__main__':


    usage = "tholdCIGen [-V] <cfg_file> <out_xml_file>"


    # setup logger

    logging.basicConfig()
    log = logging.getLogger('tholdCIGen')
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

    log.info("Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("Config file %s missing or empty" % configName)
        sys.exit(1)

    # get DAC setttings file names

    if 'dacfiles' not in sections:
        log.error("Config file %s missing [dacfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dacfiles', 'snapshot'):
        log.error("Config file %s missing [dacfiles]:snapshot option" % configName)
        sys.exit(1)

    snapshotName = configFile.get('dacfiles', 'snapshot')

    uldDacFiles = []
    lacDacFiles = []
    fleDacFiles = []
    fheDacFiles = []

    options = configFile.options('dacfiles')
    for opt in options:
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        
        if optList[0] == 'uld':
            fList = uldDacFiles
        elif optList[0] == 'lac':
            fList = lacDacFiles
        elif optList[0] == 'fle':
            fList = fleDacFiles
        elif optList[0] == 'fhe':
            fList = fheDacFiles
        else:
            continue
            
        destTwr = int(optList[1])        
        if destTwr < 0 or destTwr > 15:
            log.error("Index for [dacfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)

        value = configFile.get('dacfiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [dacfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
            
        inFile = inputFile(srcTwr, destTwr, name)
        fList.append(inFile)
        
        log.debug('Adding %s file %s to input as tower %d (from %d)', optList[0], name,
                  destTwr, srcTwr)

    # get DAC/ADC characterization file names

    if 'adcfiles' not in sections:
        log.error("Config file %s missing [adcfiles] section" % configName)
        sys.exit(1)

    uldAdcFiles = []
    lacAdcFiles = []
    fleAdcFiles = []
    fheAdcFiles = []
    pedAdcFiles = []
    biasAdcFiles = []

    options = configFile.options('adcfiles')
    for opt in options:
        
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        
        if optList[0] == 'uld2adc':
            fList = uldAdcFiles
        elif optList[0] == 'lac2adc':
            fList = lacAdcFiles
        elif optList[0] == 'fle2adc':
            fList = fleAdcFiles
        elif optList[0] == 'fhe2adc':
            fList = fheAdcFiles
        elif optList[0] == 'pedestals':
            fList = pedAdcFiles
        elif optList[0] == 'bias':
            fList = biasAdcFiles
        else:
            continue

        destTwr = int(optList[1])        
        if destTwr < 0 or destTwr > 15:
            log.error("Index for [adcfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
            
        value = configFile.get('adcfiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [adcfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)    
        inFile = inputFile(srcTwr, destTwr, name)
        fList.append(inFile)
        
        log.debug('Adding %s file %s to input as tower %d (from %d)', optList[0], name,
                  destTwr, srcTwr)

    if not configFile.has_option('adcfiles', 'intnonlin'):
        log.error("Config file %s missing [adcfiles]:intnonlin option" % configName)
        sys.exit(1)
    intNonlinName = configFile.get('adcfiles', 'intnonlin')


    # get DTD spec file names

    if 'dtdfiles' not in sections:
        log.error("Config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("Config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)

    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))


    # read snapshot DAC config file

    log.info("Reading file %s", snapshotName)
    snapshotFile = calDacXML.calSnapshotXML(snapshotName)
    fleDacData = snapshotFile.read('fle_dac')
    fheDacData = snapshotFile.read('fhe_dac')
    lacDacData = snapshotFile.read('log_acpt')
    uldDacData = snapshotFile.read('rng_uld_dac')
    config0Data = snapshotFile.read('config_0')
    tlist =  snapshotFile.getTowers()
    snapshotFile.close()

    # optionally overlay snapshot fragment config settings

    for f in uldDacFiles:

        log.info("Reading file %s", f.name)
        uldDacFile = calDacXML.calDacXML(f.name, 'rng_uld_dac')
        twrs = uldDacFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        dacData = uldDacFile.read()
        uldDacData[f.destTwr,...] = dacData[f.srcTwr,...]
        uldDacFile.close()
        
    for f in lacDacFiles:

        log.info("Reading file %s", f.name)
        lacDacFile = calDacXML.calDacXML(f.name, 'log_acpt')
        twrs = lacDacFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        dacData = lacDacFile.read()
        lacDacData[f.destTwr,...] = dacData[f.srcTwr,...]
        lacDacFile.close()

    for f in fleDacFiles:

        log.info("Reading file %s", f.name)
        fleDacFile = calDacXML.calDacXML(f.name, 'fle_dac')
        twrs = fleDacFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        dacData = fleDacFile.read()
        fleDacData[f.destTwr,...] = dacData[f.srcTwr,...]
        fleDacFile.close()

    for f in fheDacFiles:

        log.info("Reading file %s", f.name)
        fheDacFile = calDacXML.calDacXML(f.name, 'fhe_dac')
        twrs = fheDacFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        dacData = fheDacFile.read()
        fheDacData[f.destTwr,...] = dacData[f.srcTwr,...]
        fheDacFile.close()        

    # get gain indicies

    leGainData = (config0Data & 0x0007)
    heGainData = ((config0Data & 0x0078) >> 3)

    # create empty ADC data arrays

    pedData = Numeric.zeros((calConstant.NUM_TEM, 9, calConstant.NUM_RNG, calConstant.NUM_ROW,
                             calConstant.NUM_END, calConstant.NUM_FE), Numeric.Float32)
    uldAdcData = Numeric.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    lacAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    fleAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    fheAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    biasAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 2), Numeric.Float32)

    # read LAC/ADC characterization file
    
    for f in lacAdcFiles:    

        log.info("Reading file %s", f.name)
        lacAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = lacAdcFile.info()
        if i['TTYPE1'] != 'log_acpt':
            log.error("File %s not a LAC ADC file", f.name)
            sys.exit(1)
        twrs = lacAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = lacAdcFile.read()
        lacAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        lacAdcFile.close()

    # read ULD/ADC characterization files

    for f in uldAdcFiles:        
    
        log.info("Reading file %s", f.name)
        uldAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = uldAdcFile.info()
        if i['TTYPE1'] != 'rng_uld_dac':
            log.error("File %s not a ULD ADC file", f.name)
            sys.exit(1)
        twrs = uldAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = uldAdcFile.read()
        f.version = uldAdcFile.getVersion()
        uldAdcData[:,f.destTwr,...] = adcData[:,f.srcTwr,...]
        uldAdcFile.close()

    # read FLE/ADC characterization files

    for f in fleAdcFiles:
        
        log.info("Reading file %s", f.name)
        fleAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = fleAdcFile.info()
        if i['TTYPE1'] != 'fle_dac':
            log.error("File %s not a FLE ADC file", f.name)
            sys.exit(1)
        twrs = fleAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = fleAdcFile.read()
        fleAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fleAdcFile.close()
        
    # read FHE/ADC characterization files

    for f in fheAdcFiles:    

        log.info("Reading file %s", f.name)
        fheAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = fheAdcFile.info()
        if i['TTYPE1'] != 'fhe_dac':
            log.error("File %s not a FHE ADC file", f.name)
            sys.exit(1)
        twrs = fheAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = fheAdcFile.read()
        fheAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fheAdcFile.close()

    # read pedestal values file

    for f in pedAdcFiles:    

        log.info("Reading file %s", f.name)
        pedFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = pedFile.info()
        if i['TTYPE1'] != 'pedestal value':
            log.error("File %s not a PED ADC file", f.name)
            sys.exit(1)
        twrs = pedFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = pedFile.read()
        pedData[f.destTwr,...] = adcData
        pedFile.close()

    # read bias correction file

    for f in biasAdcFiles:

        log.info("Reading file %s", f.name)
        biasFile = calDacXML.calEnergyXML(f.name, 'thrBias')
        twrs = biasFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = biasFile.read()
        biasAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        biasFile.close()

    # read ADC non-linearity characterization data

    log.info("Reading file %s", intNonlinName)
    intNonlinFile = calCalibXML.calIntNonlinCalibXML(intNonlinName)
    (intNonlinLengthData, intNonlinDacData, intNonlinAdcData) = intNonlinFile.read()
    intNonlinFile.close()

    # do pedestal corrections for ULD ADC data

    for f in uldAdcFiles:
        
        # first check for ULD values that are not pedestal subtracted

        if uldAdcData[0, f.destTwr, 0, 0, 0, -1] == 4095.0:
            log.debug("Implementing pedestal subtraction for file %s, ver %d",
                      f.name, f.version)
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                        for erng in range(3):
                            if erng < calConstant.CRNG_HEX8:
                                gData = int(leGainData[f.destTwr, row, end, fe])
                            else:
                                gData = int(heGainData[f.destTwr, row, end, fe])
                                gData -= 8
                                if gData < 0:
                                   gData = 8
                            psData = pedData[f.destTwr, gData, erng, row, end, fe]
                            for dac in range(128):
                               uldAdcData[erng, f.destTwr, row, end, fe, dac] -= psData

        # correct pedestal subtraction for v1 ULD XML files
     
        elif f.version <= 1:
            log.debug("Correcting pedestal subtraction for file %s, ver %d",
                      f.name, f.version)
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                        for erng in range(3):
                            if erng < calConstant.CRNG_HEX8:
                                gData = int(leGainData[f.destTwr, row, end, fe])
                            else:
                                gData = int(heGainData[f.destTwr, row, end, fe])
                                gData -= 8
                                if gData < 0:
                                    gData = 8
                            psData = pedData[f.destTwr, gData, erng, row, end, fe]
                            paData = pedData[f.destTwr, gData, (erng + 1), row, end, fe]
                            for dac in range(128):
                               uldAdcData[erng, f.destTwr, row, end, fe, dac] += paData
                               uldAdcData[erng, f.destTwr, row, end, fe, dac] -= psData


    # do warning check of intNonlin saturation against ULD saturation

    for f in uldAdcFiles:

        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(3):

                        intData = intNonlinAdcData[erng]
                        lenData = intNonlinLengthData[erng]
                        dLen = int(lenData[f.destTwr, row, end, fe, 0])
                        id = intData[f.destTwr, row, end, fe, (dLen - 1)]
                        dac = int(uldDacData[f.destTwr, row, end, fe])
                        uld = uldAdcData[erng, f.destTwr, row, end, fe, dac]
                        if uld > id:
                            log.warning("ULD %0.3f > INTNONLIN %0.3f for T%d,%s%s,%d,%s", uld, id, f.destTwr,
                                        calConstant.CROW[row], calConstant.CPM[end], fe, calConstant.CRNG[erng])
                
                               

    # create CAL calibration output XML file

    log.info("Creating file %s", calibName)
    calibFile = calCalibXML.calTholdCICalibXML(calibName, mode = calCalibXML.MODE_CREATE)
    dacData = (uldDacData, lacDacData, fleDacData, fheDacData)
    adcData = (uldAdcData, lacAdcData, fleAdcData, fheAdcData)
    calibFile.write(dacData, adcData, intNonlinAdcData[calConstant.CRNG_HEX1], intNonlinLengthData[calConstant.CRNG_HEX1],
                    pedData, leGainData, heGainData, biasAdcData, tems = tlist)
    calibFile.close()


    # fixup calibration XML file - insert DTD info

    calCalibXML.insertDTD(calibName, dtdName)

    sys.exit(0)



