"""
Convert CAL DAC setting XML file to txt.  the commandline format is.

The txt output format is:
twr row xtal online_face dac

The commandline is
python dacXML2TXT.py FLE|FHE|LAC <dac_xml_file> 

where:
    FLE|FHE|LAC        = DAC dacType to validate
    <dac_xml_file>    = GLAST Cal dac xml 'fragment' file (minuend)
"""

__facility__    = "Offline"
__abstract__    = "Convert CAL DAC settings XML file to txt."
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/07/26 16:05:27 $"
__version__     = "$Revision: 1.3 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant

usage = "python dacDiff.py FLE|FHE|LAC <dac_xml_file>"


# check command line
if len(sys.argv) != 3:
    print usage
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
print  "; twr row xtal online_face dac"

for twr in dacTwrs:
    for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    print twr,row,end,fe,dac[twr,row,end,fe]
