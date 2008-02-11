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
__date__      = "$Date: 2008/02/03 00:51:50 $"
__version__   = "$Revision: 1.33 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import logging
import ConfigParser
import getopt

import numarray

import calDacXML
import calFitsXML
import calCalibXML
import calConstant




class inputFile:
    """
    Represents one input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name, version = None):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        Param: adcData A numarray ADC data array from the input file.
        Param: version The XML format version of the input file.
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name
        self.version = version
	


def setRange(dacs, rangeData):
    """
    Adjust DAC values based on FINE/COARSE range.
    """
    
    return numarray.where(rangeData, (dacs - 64), dacs)


def linear(c1, c0):

    adc = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE, 64), numarray.Float32)
            
    for dac in range(64):
        adc[...,dac] = ((c1 * dac) + c0).astype(numarray.Float32)
        
    return adc
    
    
def linearULD(c1, c0):

    adc = numarray.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE, 64), numarray.Float32)
            
    for dac in range(64):
        adc[...,dac] = ((c1 * dac) + c0).astype(numarray.Float32)
        
    return adc
                   



##################################################################################        



if __name__ == '__main__':



    # setup logger

    logging.basicConfig()
    log = logging.getLogger('tholdCIGen')
    log.setLevel(logging.INFO)

    # get environment settings

    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except KeyError:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)        
        
    args = opts[1]
    if len(args) != 2:
        log.error(__doc__)
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
        
    # get gain settings    
        
    if 'gains' not in sections:
        log.error("Config file %s missing [gains] section" % configName)
        sys.exit(1)
    options = configFile.options('gains')
    if 'hegain' not in options:
        log.error("Config file %s missing [gains]:hegain option" % configName)
        sys.exit(1)
    if 'legain' not in options:
        log.error("Config file %s missing [gains]:legain option" % configName)
        sys.exit(1) 
    
    leGain = int(configFile.get('gains', 'legain'))
    heGain = int(configFile.get('gains', 'hegain'))
    
    log.debug("Using LE gain %d", leGain)
    log.debug("Using HE gain %d", heGain)             

    # get DAC setttings file names

    if 'dacfiles' not in sections:
        log.error("Config file %s missing [dacfiles] section" % configName)
        sys.exit(1)
    
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
        
        # 16 tower DAC file
        
        if destTwr == 16:
        
            name = configFile.get('dacfiles', opt)
            inFile = inputFile(None, destTwr, name)
            fList.append(inFile)
            log.debug('Adding file %s', name)
           
        # single tower DAC files   
               
        else:
        
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
                  
    if 'dacslope' not in options:
        log.error("Config file %s is missing [dacfiles]:dacslope options", configName)
        sys.exit(1)
    dacSlopeName = configFile.get('dacfiles', 'dacslope')

    # get DAC/ADC characterization file names

    if 'adcfiles' not in sections:
        log.error("Config file %s missing [adcfiles] section" % configName)
        sys.exit(1)

    pedAdcFiles = []

    options = configFile.options('adcfiles')
    for opt in options:
        
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        if optList[0] == 'pedestals':
            fList = pedAdcFiles
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

    if 'bias' not in options:
        log.error("Config file %s missing [adcfiles]:bias option" % configName)
        sys.exit(1)
    biasName = configFile.get('adcfiles', 'bias')    

    if 'intnonlin' not in options:
        log.error("Config file %s missing [adcfiles]:intnonlin option" % configName)
        sys.exit(1)
    intNonlinName = configFile.get('adcfiles', 'intnonlin')

    if 'muslope' not in options:
        log.error("Config file %s missing [adcfiles]:muslope option" % configName)
        sys.exit(1)
    muSlopeName = configFile.get('adcfiles', 'muslope')

    # get DTD spec file names

    if 'dtdfiles' not in sections:
        log.error("Config file %s missing [dtdfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dtdfiles', 'dtdfile'):
        log.error("Config file %s missing [dtdfiles]:dtdfile option" % configName)
        sys.exit(1)

    dtdName = os.path.join(calibUtilRoot, 'xml', configFile.get('dtdfiles', 'dtdfile'))

  
    # read in ULD DAC settings files

    if len(uldDacFiles) == 1 and uldDacFiles[0].destTwr == 16:
    
        f = uldDacFiles[0]  
        log.info("Reading file %s", f.name)  
        uldDacFile = calDacXML.calSettingsXML(f.name, 'rng_uld_dac')
        uldDacData = uldDacFile.read()
        uldDacFile.close()
        
    else:
        
        uldDacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), numarray.UInt8)
        
        for f in uldDacFiles:

            log.info("Reading file %s", f.name)
            uldDacFile = calDacXML.calSettingsXML(f.name, 'rng_uld_dac')
            twrs = uldDacFile.getTowers()
            if f.srcTwr not in twrs:
                log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
                sys.exit(1)
            dacData = uldDacFile.read()
            uldDacData[f.destTwr,...] = dacData[f.srcTwr,...]
            uldDacFile.close()
            
    # read in LAC DAC settings files
    
    if len(lacDacFiles) == 1 and lacDacFiles[0].destTwr == 16:
    
        f = lacDacFiles[0]
        log.info("Reading file %s", f.name)
        lacDacFile = calDacXML.calSettingsXML(f.name, 'log_acpt')
        lacDacData = lacDacFile.read()
        lacDacFile.close()
    
    else:
    
        lacDacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), numarray.UInt8)    
    
        for f in lacDacFiles:

            log.info("Reading file %s", f.name)
            lacDacFile = calDacXML.calSettingsXML(f.name, 'log_acpt')
            twrs = lacDacFile.getTowers()
            if f.srcTwr not in twrs:
                log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
                sys.exit(1)
            dacData = lacDacFile.read()
            lacDacData[f.destTwr,...] = dacData[f.srcTwr,...]
            lacDacFile.close()

    # read in FLE DAC settings files
    
    if len(fleDacFiles) == 1 and fleDacFiles[0].destTwr == 16:
    
        f = fleDacFiles[0]
        log.info("Reading file %s", f.name)
        fleDacFile = calDacXML.calSettingsXML(f.name, 'fle_dac')
        fleDacData = fleDacFile.read()
        fleDacFile.close()

    else:
        
        fleDacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), numarray.UInt8)
        
        for f in fleDacFiles:

            log.info("Reading file %s", f.name)
            fleDacFile = calDacXML.calSettingsXML(f.name, 'fle_dac')
            twrs = fleDacFile.getTowers()
            if f.srcTwr not in twrs:
                log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
                sys.exit(1)
            dacData = fleDacFile.read()
            fleDacData[f.destTwr,...] = dacData[f.srcTwr,...]
            fleDacFile.close()
        
    # read in FHE DAC settings files
    
    if len(fheDacFiles) == 1 and fheDacFiles[0].destTwr == 16:
    
        f = fheDacFiles[0]
        log.info("Reading file %s", f.name)
        fheDacFile = calDacXML.calSettingsXML(f.name, 'fhe_dac')
        fheDacData = fheDacFile.read()
        fheDacFile.close()
        
    else:
    
        fheDacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), numarray.UInt8)    

        for f in fheDacFiles:

            log.info("Reading file %s", f.name)
            fheDacFile = calDacXML.calSettingsXML(f.name, 'fhe_dac')
            twrs = fheDacFile.getTowers()
            if f.srcTwr not in twrs:
                log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
                sys.exit(1)
            dacData = fheDacFile.read()
            fheDacData[f.destTwr,...] = dacData[f.srcTwr,...]
            fheDacFile.close() 
            
    # read DAC/Energy conversion file
    
    log.info("Reading file %s", dacSlopeName)
    dacSlopeFile = calCalibXML.calDacSlopesCalibXML(dacSlopeName)
    dacSlopeData = dacSlopeFile.read()
    dacSlopeFile.close()       

    # get gain indicies

    leGainData = numarray.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), numarray.UInt8)
    heGainData = numarray.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), numarray.UInt8)
    leGainData = leGainData * leGain
    heGainData = heGainData * heGain    

    # create empty ADC data arrays

    pedData = numarray.zeros((calConstant.NUM_TEM, 9, calConstant.NUM_RNG, calConstant.NUM_ROW,
                             calConstant.NUM_END, calConstant.NUM_FE), numarray.Float32)

    # read pedestal values files

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
        pedData[f.destTwr,...] = adcData[:]
        pedFile.close()

    # read bias correction file

    log.info("Reading file %s", biasName)
    biasFile = calDacXML.calEnergyXML(biasName, 'thrBias')
    biasAdcData = biasFile.read()
    tlist = biasFile.getTowers()
    biasFile.close()

    # read ADC non-linearity characterization data

    log.info("Reading file %s", intNonlinName)
    intNonlinFile = calCalibXML.calIntNonlinCalibXML(intNonlinName)
    (intNonlinLengthData, intNonlinDacData, intNonlinAdcData) = intNonlinFile.read()
    intNonlinFile.close()
    
    # read muSlope ADC/Energy conversion file
    
    log.info("Reading file %s", muSlopeName)
    muSlopeFile = calCalibXML.calMuSlopeCalibXML(muSlopeName)
    muSlopeData = muSlopeFile.read()
    muSlopeFile.close()
    
    # convert FHE thresholds from MeV to HEX8 ADC counts
    
    c1 = dacSlopeData[0][...,4]
    c0 = dacSlopeData[0][...,5]
    eng = linear(c1, c0)
    gain = muSlopeData[...,calConstant.CRNG_HEX8,0]
    bias = biasAdcData[...,1]
    adc = (eng / gain[...,numarray.NewAxis]) - bias[...,numarray.NewAxis]
    fheAdcData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 128), numarray.Float32)
    fheAdcData[...,0:64] = adc[:]
    fheAdcData[...,64:128] = adc[:]
    fheAdcData = numarray.clip(fheAdcData, 0, 4095)
    log.debug("FHE ADC\n%s", fheAdcData[0,0,0,0,:])
    
    # convert FLE thresholds from MeV to LEX8 ADC counts
    
    c1 = dacSlopeData[0][...,2]
    c0 = dacSlopeData[0][...,3]
    eng = linear(c1, c0)
    gain = muSlopeData[...,calConstant.CRNG_LEX1,0]
    bias = biasAdcData[...,0]
    adc = (eng / gain[...,numarray.NewAxis])
    scale = (gain / muSlopeData[...,calConstant.CRNG_LEX8,0])
    adc = (adc * scale[...,numarray.NewAxis]) - bias[...,numarray.NewAxis]
    fleAdcData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 128), numarray.Float32)
    fleAdcData[...,0:64] = adc[:]
    fleAdcData[...,64:128] = adc[:]
    fleAdcData = numarray.clip(fleAdcData, 0, 0xffff)
    log.debug("FLE ADC\n%s", fleAdcData[0,0,0,0,:])
    
    # convert LAC thresholds from MeV to LEX8 ADC counts
    
    c1 = dacSlopeData[0][...,0]
    c0 = dacSlopeData[0][...,1]
    eng = linear(c1, c0)
    gain = muSlopeData[...,calConstant.CRNG_LEX8,0]
    adc = (eng / gain[...,numarray.NewAxis])
    lacAdcData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 128), numarray.Float32)
    lacAdcData[...,0:64] = adc[:]
    lacAdcData[...,64:128] = adc[:]
    lacAdcData = numarray.clip(lacAdcData, 0, 4095)
    log.debug("LAC ADC\n%s", lacAdcData[0,0,0,0,:])
    
    # convert ULD thresholds from MeV to LEX8, LEX1, HEX8 ADC counts
    
    c1 = dacSlopeData[1][...,0]
    c0 = dacSlopeData[1][...,1]
    eng = linearULD(c1, c0)
    
    uldAdcData = numarray.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 128), numarray.Float32)
    
    # ULD LEX8
    
    gain = muSlopeData[...,calConstant.CRNG_LEX8,0]
    adc = (eng[calConstant.CRNG_LEX8,...] / gain[...,numarray.NewAxis])
    satValue = (dacSlopeData[1][calConstant.CRNG_LEX8,...,2] / gain)
    satValue = numarray.reshape(numarray.repeat(satValue, 64, -1), (calConstant.NUM_TEM, 
        calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE, 64))
    satMask = (adc > satValue)
    adc = numarray.where(satMask, satValue, adc)
    uldAdcData[calConstant.CRNG_LEX8,...,0:64] = adc[:]
    uldAdcData[calConstant.CRNG_LEX8,...,64:128] = adc[:] 
    
    # ULD LEX1
    
    gain = muSlopeData[...,calConstant.CRNG_LEX1,0]
    adc = (eng[calConstant.CRNG_LEX1,...] / gain[...,numarray.NewAxis])
    satValue = (dacSlopeData[1][calConstant.CRNG_LEX1,...,2] / gain)
    satValue = numarray.reshape(numarray.repeat(satValue, 64, -1), (calConstant.NUM_TEM, 
        calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE, 64))
    satMask = (adc > satValue)
    adc = numarray.where(satMask, satValue, adc)
    uldAdcData[calConstant.CRNG_LEX1,...,0:64] = adc[:]
    uldAdcData[calConstant.CRNG_LEX1,...,64:128] = adc[:]
        
    # ULD HEX8
    
    gain = muSlopeData[...,calConstant.CRNG_HEX8,0]
    adc = (eng[calConstant.CRNG_HEX8,...] / gain[...,numarray.NewAxis])
    satValue = (dacSlopeData[1][calConstant.CRNG_HEX8,...,2] / gain)
    satValue = numarray.reshape(numarray.repeat(satValue, 64, -1), (calConstant.NUM_TEM, 
        calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE, 64))
    satMask = (adc > satValue)
    adc = numarray.where(satMask, satValue, adc)
    uldAdcData[calConstant.CRNG_HEX8,...,0:64] = adc[:]
    uldAdcData[calConstant.CRNG_HEX8,...,64:128] = adc[:]
    
    uldAdcData = numarray.clip(uldAdcData, 0, 4095)
    
    log.debug("ULD ADC\n%s", uldAdcData[:,0,0,0,0,:])

    # create CAL calibration output XML file

    log.info("Creating file %s", calibName)
    calibFile = calCalibXML.calTholdCICalibXML(calibName, mode = calCalibXML.MODE_CREATE)
    
    doc = calibFile.getDoc()
    c = doc.createComment("Input LE gain parameter = %d" % leGain)
    doc.appendChild(c)
    c = doc.createComment("Input HE gain parameter = %d" % heGain)
    doc.appendChild(c)
    for f in uldDacFiles:
        c = doc.createComment("Input ULD DAC settings file = %s" % os.path.basename(f.name))
        doc.appendChild(c)
    for f in fleDacFiles:
        c = doc.createComment("Input FLE DAC settings file = %s" % os.path.basename(f.name))
        doc.appendChild(c)
    for f in fheDacFiles:
        c = doc.createComment("Input FHE DAC settings file = %s" % os.path.basename(f.name))
        doc.appendChild(c) 
    for f in lacDacFiles:
        c = doc.createComment("Input LAC DAC settings file = %s" % os.path.basename(f.name)) 
        doc.appendChild(c) 
    for f in pedAdcFiles:
        c = doc.createComment("Input pedestal value file = %s" % os.path.basename(f.name))
        doc.appendChild(c) 
    c = doc.createComment("Input bias value file = %s" % os.path.basename(biasName))
    doc.appendChild(c)     
    c = doc.createComment("Input IntNonlin file = %s" % os.path.basename(intNonlinName))
    doc.appendChild(c)
    c = doc.createComment("Input DacSlope file = %s" % os.path.basename(dacSlopeName))
    doc.appendChild(c)
    c = doc.createComment("Input MuSlope file = %s" % os.path.basename(muSlopeName))
    doc.appendChild(c)                         
    
    dacData = (uldDacData, lacDacData, fleDacData, fheDacData)
    adcData = (uldAdcData, lacAdcData, fleAdcData, fheAdcData)
    calibFile.write(dacData, adcData, intNonlinAdcData[calConstant.CRNG_HEX1], intNonlinLengthData[calConstant.CRNG_HEX1],
                    pedData, leGainData, heGainData, biasAdcData, tems = tlist)
    calibFile.close()


    # fixup calibration XML file - insert DTD info

    calCalibXML.insertDTD(calibName, dtdName)

    sys.exit(0)



