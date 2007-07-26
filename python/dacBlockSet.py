"""
Write same value to blocks of DAC settings in online DAC setting xml file

The commandline is
python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC <val> <input_xml> <output_xml>

where:
    FLE|FHE|LAC       = DAC data type
    -f <face>         = face = POS|NEG overwrite all values w/ given face
    <input_xml>       = input dac settings xml
    <output_xml>      = output dac settings xml

"""

__facility__    = "Offline"
__abstract__    = "Write same value to blocks of DAC settings in online DAC setting xml file "
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/02/26 23:15:57 $"
__version__     = "$Revision: 1.2 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

import sys
import calDacXML
import calConstant
import string
import logging
import getopt

usage = "python dacBlockSet.py  [-f POS|NEG] FLE|FHE|LAC <val> <input_xml> <output_xml>"

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
(dacType, val, inPath, outPath) = args


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
dacFile = calDacXML.calDacXML(inPath, dacType)

# get active towers
dacTwrs = dacFile.getTowers()
print dacTwrs

# load up arrays
dac = dacFile.read()

# overwrite single face
if overwriteFace:
    for twr in dacTwrs:
        online_face = calConstant.name_to_online_face[face]
        dac[twr,:,online_face,:] = int(val)

# output new file
outFile = calDacXML.calDacXML(outPath, dacType, calDacXML.MODE_CREATE)
outFile.write(dac, filename=outPath,tems=dacTwrs)
outFile.close()

sys.exit(0)

    
