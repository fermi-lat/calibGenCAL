"""
calCalibGen.py
"""

import sys, os
import logging
import ConfigParser
import getopt

import Numeric

import calDacXML
import calFitsXML
import calCalibXML

from calExcept import *


usage = "calCalibGen <cfg_file> <out_xml_file>"


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
calibName = args[1]


# read config file settings

log.info("calCalibGen: Reading file %s", configName)
configFile = ConfigParser.SafeConfigParser()
configFile.read(configName)
sections = configFile.sections()
if len(sections) == 0:
    raise calFileOpenExcept, "config file %s missing or empty" % configName

# get DAC setttings file names

if 'dacfiles' not in sections:
    raise calFileReadExcept, "config file %s missing [dacfiles] section" % configName
if not configFile.has_option('dacfiles', 'uldfile'):
    raise calFileReadExcept, "config file %s missing [dacfiles]:uldfile option" % configName
if not configFile.has_option('dacfiles', 'lacfile'):
    raise calFileReadExcept, "config file %s missing [dacfiles]:lacfile option" % configName
if not configFile.has_option('dacfiles', 'flefile'):
    raise calFileReadExcept, "config file %s missing [dacfiles]:flefile option" % configName
if not configFile.has_option('dacfiles', 'fhefile'):
    raise calFileReadExcept, "config file %s missing [dacfiles]:fhefile option" % configName

uldDacName = configFile.get('dacfiles', 'uldfile')
lacDacName = configFile.get('dacfiles', 'lacfile')
fleDacName = configFile.get('dacfiles', 'flefile')
fheDacName = configFile.get('dacfiles', 'fhefile')

# get DAC/ADC characterization file names

if 'adcfiles' not in sections:
    raise calFileReadExcept, "config file %s missing [adcfiles] section" % configName
if not configFile.has_option('adcfiles', 'uld2adcfile'):
    raise calFileReadExcept, "config file %s missing [adcfiles]:uld2adcfile option" % configName
if not configFile.has_option('adcfiles', 'lac2adcfile'):
    raise calFileReadExcept, "config file %s missing [adcfiles]:lac2adcfile option" % configName
if not configFile.has_option('adcfiles', 'fle2adcfile'):
    raise calFileReadExcept, "config file %s missing [adcfiles]:fle2adcfile option" % configName
if not configFile.has_option('adcfiles', 'fhe2adcfile'):
    raise calFileReadExcept, "config file %s missing [adcfiles]:fhe2adcfile option" % configName

uldAdcName = configFile.get('adcfiles', 'uld2adcfile')
lacAdcName = configFile.get('adcfiles', 'lac2adcfile')
fleAdcName = configFile.get('adcfiles', 'fle2adcfile')
fheAdcName = configFile.get('adcfiles', 'fhe2adcfile')

# get pedestal value file names

if 'pedfiles' not in sections:
    raise calFileReadExcept, "config file %s missing [pedfiles] section" % configName
if not configFile.has_option('pedfiles', 'pedfile'):
    raise calFileReadExcept, "config file %s missing [pedfiles]:pedfile option" % configName

pedName = configFile.get('pedfiles', 'pedfile')

# get DTD spec file names

if 'dtdfiles' not in sections:
    raise calFileReadExcept, "config file %s missing [dtdfiles] section" % configName
if not configFile.has_option('dtdfiles', 'dtdfile'):
    raise calFileReadExcept, "config file %s missing [dtdfiles]:dtdfile option" % configName

dtdName = configFile.get('dtdfiles', 'dtdfile')


# read FLE DAC config file

log.info("calCalibGen: Reading file %s", fleDacName)
fleDacFile = calDacXML.calDacXML(fleDacName, 'fle_dac')
fleDacData = fleDacFile.read()
fleDacFile.close()

# read FHE DAC config file

log.info("calCalibGen: Reading file %s", fheDacName)
fheDacFile = calDacXML.calDacXML(fheDacName, 'fhe_dac')
fheDacData = fheDacFile.read()
fheDacFile.close()

# read LAC DAC config file

log.info("calCalibGen: Reading file %s", lacDacName)
lacDacFile = calDacXML.calDacXML(lacDacName, 'log_acpt')
lacDacData = lacDacFile.read()
lacDacFile.close()

# read ULD DAC config file

log.info("calCalibGen: Reading file %s", uldDacName)
uldDacFile = calDacXML.calDacXML(uldDacName, 'rng_uld_dac')
uldDacData = uldDacFile.read()
uldDacFile.close()

# read LAC/ADC characterization file

log.info("calCalibGen: Reading file %s", lacAdcName)
lacAdcFile = calFitsXML.calFitsXML(fileName = lacAdcName, mode = calFitsXML.MODE_READONLY)
lacAdcData = lacAdcFile.read()
lacAdcFile.close()

# read ULD/ADC characterization file

log.info("calCalibGen: Reading file %s", uldAdcName)
uldAdcFile = calFitsXML.calFitsXML(fileName = uldAdcName, mode = calFitsXML.MODE_READONLY)
uldAdcData = uldAdcFile.read()
uldAdcInfo = uldAdcFile.info()
uldAdcFile.close()

# read FLE/ADC characterization file

log.info("calCalibGen: Reading file %s", fleAdcName)
fleAdcFile = calFitsXML.calFitsXML(fileName = fleAdcName, mode = calFitsXML.MODE_READONLY)
fleAdcData = fleAdcFile.read()
fleAdcFile.close()

# read FHE/ADC characterization file

log.info("calCalibGen: Reading file %s", fheAdcName)
fheAdcFile = calFitsXML.calFitsXML(fileName = fheAdcName, mode = calFitsXML.MODE_READONLY)
fheAdcData = fheAdcFile.read()
fheAdcFile.close()

# read pedestal values file

log.info("calCalibGen: Reading file %s", pedName)
pedFile = calFitsXML.calFitsXML(fileName = pedName, mode = calFitsXML.MODE_READONLY)
pedData = pedFile.read()
pedFile.close()

# create CAL calibration output XML file

lrefGain = int(uldAdcInfo['LREFGAIN'])
hrefGain = int(uldAdcInfo['HREFGAIN'])

log.info("calCalibGen: Creating file %s", calibName)
calibFile = calCalibXML.calTholdCICalibXML(calibName, mode = calCalibXML.MODE_CREATE)
dacData = (uldDacData, lacDacData, fleDacData, fheDacData)
adcData = (uldAdcData, lacAdcData, fleAdcData, fheAdcData)
calibFile.write(dacData, adcData, pedData, lrefGain, hrefGain)
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

sys.exit(0)



