"""
Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file

Method masks all settings by default.
Additional masks are ANDed together to select some region of ca.
All indexes are in offline units


The commandline is
python dacBlockSet.py  [-t twr] [-l lyr] [-c col] -R pct [-f POS|NEG]  FLE|FHE|LAC|ULD  <input_xml> <val_src> <output_xml>

where:
    FLE|FHE|LAC|ULD   = DAC data type
    -t <twr>        = mask single twr (0-15)
    -l <lyr>          = mask single layer (0-7)   [Offline indexing]
    -c <col>          = mask single column (0-11)
    -f <face>         = mask only POS or NEG xtal face
    -R <rnd_pct>      = create random mask covering rnd_pct percent of settings
    <input_xml>       = input dac settings xml
    <val_src>         = if <val_src> is numeric, then set all masked values to val_src ... else open filename <val_src> & read masked values from that file
    <output_xml>      = output dac settings xml

"""

__facility__    = "Offline"
__abstract__    = "Override a block of DAC settings in online DAC setting xml file with either constant value or values from 2nd file"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/09/04 14:46:42 $"
__version__     = "$Revision: 1.5 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calCalibXML
import calConstant
import string
import logging
import getopt
import cgc_util
import Numeric

usage = "python dacBlockSet.py  [-t twr] [-l lyr] [-c col] [-f POS|NEG][-R pct] FLE|FHE|LAC|ULD <input_xml> <val_src> <output_xml>"

# setup logger

logging.basicConfig()
log = logging.getLogger('dacBlockSet')
log.setLevel(logging.INFO)

# cfg values
owriteTwr  = False
owriteLyr  = False
owriteCol  = False
owriteFace = False
owriteRnd = False

# check command line

try:
    OptsArgs = getopt.getopt(sys.argv[1:], "t:l:c:f:R:")
except getopt.GetoptError:
    log.error("getopt error")
    log.error(usage)
    sys.exit(1)


(opts,args) = OptsArgs

for oa in opts:
    (opt,arg) = oa
    if opt == '-t':
        owriteTwr = True
        twr = int(arg)
        if twr < 0 or twr > 15:
            log.error("inavlid twr #: %d"%twr)
            sys.exit(-1)
    elif opt == '-l':
        owriteLyr = True
        lyr = int(arg)
        if (lyr < 0 or lyr > 7):
            log.error("inavlid layer #: %d"%lyr)
            sys.exit(-1)
    elif opt == '-c':
        owriteCol = True
        col = int(arg)
        if (col < 0 or col > 11):
            log.error("inavlid column #: %d"%col)
            sys.exit(-1)
    elif opt == '-f':
        owriteFace = True
        face = string.upper(arg)
    elif opt == '-R':
        owriteRnd = True
        rnd_pct = float(arg)
        if (rnd_pct < 0 or rnd_pct > 100):
            log.error("invalid percent: %f"%rnd_pct)
            sys.exit(-1)


#destination variables
if len(args) != 4:
    log.error("bad n args: " + str(len(args)))
    log.error(usage)
    sys.exit(1)
    
# get arg values
(dacType, inPath, val_src, outPath) = args


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
# first enable all channels
msk = Numeric.ones(dac.shape)

# apply twr mask
if owriteTwr:
    # build new mask to be 'AND'ed
    tmp_msk = Numeric.zeros(dac.shape)
    tmp_msk[twr,...] = 1

    # and tmp_msk with current mask
    msk &= tmp_msk

# apply layer mask
if owriteLyr:
    row = calCalibXML.layerToRow(int(lyr))
    # build new mask to be 'AND'ed
    tmp_msk = Numeric.zeros(dac.shape)
    tmp_msk[:] = 0
    tmp_msk[:,row,...] = 1

    # and tmp_msk with current mask
    msk &= tmp_msk

# apply column mask
if owriteCol:
    # build new mask to be 'AND'ed
    tmp_msk = Numeric.zeros(dac.shape)
    tmp_msk[:] = 0
    tmp_msk[...,col] = 1

    # and tmp_msk with current mask
    msk = msk & tmp_msk


# apply face mask
if owriteFace:
    online_face = calConstant.name_to_online_face[face]
    # build new mask to be 'AND'ed
    tmp_msk = Numeric.zeros(dac.shape)
    tmp_msk[:] = 0
    tmp_msk[...,online_face,:] = 1

    # and tmp_msk with current msk
    msk &= tmp_msk

# apply random mask
if owriteRnd:
    import MLab # included w/ Numeric, provides rand() method
    tmp_msk = MLab.rand(*dac.shape) < rnd_pct / 100

    # and tmp_msk with current msk
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

    
