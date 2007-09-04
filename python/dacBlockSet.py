"""
Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file

Method masks all settings by default.
Additional masks are ANDed together to select some region of ca.


The commandline is
python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC|ULD <val_src> <input_xml> <output_xml>

where:
    FLE|FHE|LAC|ULD   = DAC data type
    -f <face>         = face = mask only POS or NEG xtal face
    <input_xml>       = input dac settings xml
    <output_xml>      = output dac settings xml
    <val_src>         = if <val_src> is numeric, then set all masked values to val_src ... else open filename <val_src> & read masked values from that file

"""

__facility__    = "Offline"
__abstract__    = "Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/08/30 21:15:56 $"
__version__     = "$Revision: 1.4 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant
import string
import logging
import getopt
import cgc_util
import Numeric

usage = "python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC|ULD <val_src> <input_xml> <output_xml>"

# setup logger

logging.basicConfig()
log = logging.getLogger('dacBlockSet')
log.setLevel(logging.INFO)

# cfg values
owriteFace = False

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
        owriteFace = True
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
elif dacType == 'ULD':
    dacType = 'rng_uld_dac'
else:
    log.error("DAC dacType %s not supported" % dacType)
    sys.exit(1)

## LOAD INPUT DATA ##
# read in dac xml files
dacFile = calDacXML.calSettingsXML(inPath, dacType)

# load up arrays
dac = dacFile.read()
dacTwrs = dacFile.getTowers()

## BUILD MASK ##
# should be same shape as original data
msk = dac.copy()
# first enable all channels
msk[:] = 1

# apply face mask
if owriteFace:
    online_face = calConstant.name_to_online_face[face]
    # build new mask to be 'AND'ed
    tmp_msk = dac.copy()
    tmp_msk[:] = 0
    tmp_msk[...,online_face,:] = 1

    # and new_msk with current msk
    msk &= tmp_msk


## LOAD OWRITE DATA ##
# constants
if cgc_util.isNumber(val_src):
    owrite_val = int(val_src)
else:
    owrite_dacfile = calDacXML.calSettingsXML(val_src, dacType)
    owrite_val = owrite_dacfile.read()

# choose output value based on mask
dac = Numeric.choose(msk,(dac,owrite_val))

# output new file
outFile = calDacXML.calSettingsXML(outPath, dacType, calDacXML.MODE_CREATE)
outFile.write(dac, tems=dacTwrs)
outFile.close()

sys.exit(0)

    
