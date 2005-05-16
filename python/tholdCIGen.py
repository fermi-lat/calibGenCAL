"""
Tool to produce CAL TholdCI XML calibration data files.
"""

import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calDacXML
import calFitsXML
import calCalibXML



if __name__ == '__main__':


    usage = "genTholdCI <cfg_file> <out_xml_file>"


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
        opts = getopt.getopt(sys.argv[1:], "")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)
        
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
        log.error("config file %s missing or empty" % configName)
        sys.exit(1)

    # get DAC setttings file names

    if 'dacfiles' not in sections:
        log.error("config file %s missing [dacfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('dacfiles', 'snapshot'):
        log.error("config file %s missing [dacfiles]:snapshot option" % configName)
        sys.exit(1)

    snapshotName = configFile.get('dacfiles', 'snapshot')


    # get DAC/ADC characterization file names

    if 'adcfiles' not in sections:
        log.error("config file %s missing [adcfiles] section" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'uld2adc'):
        log.error("config file %s missing [adcfiles]:uld2adcfile option" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'lac2adc'):
        log.error("config file %s missing [adcfiles]:lac2adcfile option" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'fle2adc'):
        log.error("config file %s missing [adcfiles]:fle2adcfile option" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'fhe2adc'):
        log.error("config file %s missing [adcfiles]:fhe2adcfile option" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'intnonlin'):
        log.error("config file %s missing [adcfiles]:intnonlin option" % configName)
        sys.exit(1)
    if not configFile.has_option('adcfiles', 'pedestals'):
        log.error("config file %s missing [adcfiles]:pedestals option" % configName)
        sys.exit(1)    

    uldAdcName = configFile.get('adcfiles', 'uld2adc')
    lacAdcName = configFile.get('adcfiles', 'lac2adc')
    fleAdcName = configFile.get('adcfiles', 'fle2adc')
    fheAdcName = configFile.get('adcfiles', 'fhe2adc')
    intNonlinName = configFile.get('adcfiles', 'intnonlin')
    pedName = configFile.get('adcfiles', 'pedestals')



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
    snapshotFile.close()

    # get gain indicies

    leGainData = (config0Data & 0x0007)
    heGainData = ((config0Data & 0x0078) >> 3)

    # read LAC/ADC characterization file

    log.info("genTholdCI: Reading file %s", lacAdcName)
    lacAdcFile = calFitsXML.calFitsXML(fileName = lacAdcName, mode = calFitsXML.MODE_READONLY)
    lacAdcData = lacAdcFile.read()
    lacAdcFile.close()

    # read ULD/ADC characterization file

    log.info("genTholdCI: Reading file %s", uldAdcName)
    uldAdcFile = calFitsXML.calFitsXML(fileName = uldAdcName, mode = calFitsXML.MODE_READONLY)
    uldAdcData = uldAdcFile.read()
    uldAdcFile.close()

    # read FLE/ADC characterization file

    log.info("genTholdCI: Reading file %s", fleAdcName)
    fleAdcFile = calFitsXML.calFitsXML(fileName = fleAdcName, mode = calFitsXML.MODE_READONLY)
    fleAdcData = fleAdcFile.read()
    fleAdcFile.close()

    # read FHE/ADC characterization file

    log.info("genTholdCI: Reading file %s", fheAdcName)
    fheAdcFile = calFitsXML.calFitsXML(fileName = fheAdcName, mode = calFitsXML.MODE_READONLY)
    fheAdcData = fheAdcFile.read()
    fheAdcFile.close()

    # read ADC non-linearity characterization data

    log.info("genTholdCI: Reading file %s", intNonlinName)
    intNonlinFile = calCalibXML.calIntNonlinCalibXML(intNonlinName)
    (intNonlinDacData, intNonlinAdcData) = intNonlinFile.read()
    intNonlinFile.close()

    # read pedestal values file

    log.info("genTholdCI: Reading file %s", pedName)
    pedFile = calFitsXML.calFitsXML(fileName = pedName, mode = calFitsXML.MODE_READONLY)
    pedData = Numeric.zeros((16, 9, 4, 8, 2, 12), Numeric.Float32)
    pedData[0,:] = pedFile.read()
    pedFile.close()

    # create CAL calibration output XML file

    log.info("genTholdCI: Creating file %s", calibName)
    calibFile = calCalibXML.calTholdCICalibXML(calibName, mode = calCalibXML.MODE_CREATE)
    dacData = (uldDacData, lacDacData, fleDacData, fheDacData)
    adcData = (uldAdcData, lacAdcData, fleAdcData, fheAdcData)
    calibFile.write(dacData, adcData, intNonlinAdcData[3], pedData, leGainData, heGainData)
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



