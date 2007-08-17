"""
Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file

The commandline is
python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC <val_src> <input_xml> <output_xml>

where:
    FLE|FHE|LAC       = DAC data type
    -f <face>         = face = POS|NEG overwrite all values w/ given face
    <input_xml>       = input dac settings xml
    <output_xml>      = output dac settings xml
    <val_src>         = if <val_src> is numeric, then set all masked values to val_src ... else open filename <val_src> & read masked values from that file

"""

__facility__    = "Offline"
__abstract__    = "Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/07/26 21:11:37 $"
__version__     = "$Revision: 1.2 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant
import string
import logging
import getopt
import cgc_util

usage = "python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC <val_src> <input_xml> <output_xml>"

# setup logger

logging.basicConfig()
log = logging.getLogger('dacBlockSet')
log.setLevel(logging.INFO)


# check command line

try:
    OptsArgs = getopt.getopt(sys.argv[1:], "f:")
except getopt.GetoptError:
    log.error("getopt error")
    log.error(usage)
    sys.exit(1)


(opts,args) = OptsArgs

for oa in opts:
    (opt,arg) = oa
    if opt == '-f':
        overwriteFace = True
        face = string.upper(arg)

#destination variables
if len(args) != 4:
    log.error("bad n args: " + str(len(args)))
    log.error(usage)
    sys.exit(1)
    
# get arg values
(dacType, val_src, inPath, outPath) = args


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

## LOAD INPUT DATA ##
# read in dac xml files
dacFile = calDacXML.calSettingsXML(inPath, dacType)

# get active towers
dacTwrs = set(dacFile.getTowers())

# load up arrays
dac = dacFile.read()

## LOAD OWRITE DATA ##
# constants
OWRITE_MODE_VAL = 0
OWRITE_MODE_FILE = 1
if cgc_util.isNumber(val_src):
    owrite_mode = OWRITE_MODE_VAL
else:
    owrite_mode = OWRITE_MODE_FILE
    owrite_dacfile = calDacXML.calSettingsXML(val_src, dacType)
    owrite_twrs = set(owrite_dacfile.getTowers())
    owrite_dac = owrite_dacfile.read()
    # only overwrite towers which both files have in common
    dacTwrs = dacTwrs.intersection(owrite_twrs)

# overwrite single face
for twr in dacTwrs:
    online_face = calConstant.name_to_online_face[face]
    if owrite_mode == OWRITE_MODE_VAL:
        dac[twr,:,online_face,:] = int(val_src)
    else:
        dac[twr,:,online_face,:] = owrite_dac[twr,:,online_face,:]


# output new file
outFile = calDacXML.calSettingsXML(outPath, dacType, calDacXML.MODE_CREATE)
outFile.write(dac, tems=dacTwrs)
outFile.close()

sys.exit(0)

    
