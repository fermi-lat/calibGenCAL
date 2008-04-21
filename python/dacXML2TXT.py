"""
Convert CAL DAC setting XML file to txt.  the commandline format is.

The txt output format is:
twr offline_lyr offline_face column dac

The commandline is
python dacXML2TXT.py FLE|FHE|LAC|ULD <dac_xml_file> 

where:
    FLE|FHE|LAC|ULD        = DAC dacType to convert
    <dac_xml_file>    = GLAST Cal dac xml 'fragment' file (minuend)
"""

__facility__    = "Offline"
__abstract__    = "Convert CAL DAC settings XML file to txt."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2008/02/04 20:14:59 $"
__version__     = "$Revision: 1.7 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant
import calCalibXML


# check command line
if len(sys.argv) != 3:
    print __doc__
    sys.exit(1)


# get filenames
dacType  = sys.argv[1]
dacPath = sys.argv[2]

#determine dac dacType
if dacType == 'FLE':
    dacType = 'fle_dac'
elif dacType == 'FHE':
    dacType = 'fhe_dac'
elif dacType == 'LAC':
    dacType = 'log_acpt'
elif dacType == 'ULD':
    dacType = 'rng_uld_dac'
else:
    print "DAC dacType %s not supported" % dacType
    sys.exit(1)


# read in dac xml files
dacFile = calDacXML.calSettingsXML(dacPath, dacType)

# check that towers are the same (ok if they are, just print warning)
dacTwrs = dacFile.getTowers()

# load up arrays
dac = dacFile.read()

#print header as comment
print  ";twr row online_face column dac"

for twr in dacTwrs:
    for offline_lyr in range(calConstant.NUM_ROW):
            for offline_face in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    online_row = calCalibXML.layerToRow(offline_lyr)
                    online_end = calConstant.offline_face_to_online[offline_face]
                    print twr, offline_lyr, fe, offline_face, dac[twr, online_row, online_end, fe]
