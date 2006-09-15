"""
Ped subtract a CIDAC2ADC curve.  For each channel, subtract adc0 from every adc value in curve

python inlPedSubtract.py <input_xml> <output_xml>

where:
    <input_xml>    = input GLAST Cal cidac2adc offline calib file
    <output_xml>    = output GLAST Cal cidac2adc offline calib file
"""

__facility__    = "Offline"
__abstract__    = "Pedestal subtract CAL CIDAC2ADC XML file."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2006/09/12 19:34:25 $"
__version__     = "$Revision: 1.1 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys,os
import calCalibXML
import calConstant
import logging
import zachUtil
import array
import Numeric

usage = "Usage: python inlPedSubtract.py <input_xml> <output_xml>"

# setup logger
logging.basicConfig()
log = logging.getLogger('inlPedSubtract')
log.setLevel(logging.INFO)

# get environment settings
try:
    calibUtilRoot = os.environ['CALIBUTILROOT']
except:
    log.error('CALIBUTILROOT must be defined')
    sys.exit(1)
dtdName = "calCalib_v2r3.dtd" #default value
dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

# check command line
if len(sys.argv) != 3:
    log.error("%d args found."%len(sys.argv))
    log.error(usage)
    sys.exit(1)

# get filenames
inPath = sys.argv[1]
outPath = sys.argv[2]

# read in dac xml files
log.info("Opening %s"%inPath)
inlFile = calCalibXML.calIntNonlinCalibXML(inPath)
twrSet = inlFile.getTowers()

# load up arrays
log.info("Reading %s"%inPath)
inl = inlFile.read()

(inlLen, inlDAC, inlADC) = inl

# from calCalibXML doc
# adcData -   A list of 4 elements, each a reference to a Numeric
# array of ADC values. The shape of each array is
# (16, 8, 2, 12, 256).  The last dimension contains the
# ADC values.  The number of valid values is determined
# by the corresponding value from the lengthData arrays.


# subtract 1st point from each ADC list
newADC = []
for rngData in inlADC:
    rngData = rngData - Numeric.reshape(rngData[...,0],(16,8,2,12,1))
    newADC.append(rngData)

# write new output file
outFile = calCalibXML.calIntNonlinCalibXML(outPath, calCalibXML.MODE_CREATE)
outFile.write(inlLen, inlDAC, newADC, tems=twrSet)
calCalibXML.insertDTD(outPath, dtdPath)

sys.exit(0)
