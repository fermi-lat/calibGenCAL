"""
Convert neighbor xtalk curves from (src=LECIDAC_CGON, dest=HEX8ADC) to (src=HEDAC_SCINT_CGOFF, dest=HEDAC_SCINT_CGOFF)
output is same format as neighborXtalk txt

python dacConvertNeighborXtalk.py <neighborXtalk.txt> <calibGain.txt> <inlCgOff.xml> <mevPerDAC.xml> <asym.xml> <outputROOTfile>

where:
       <neighborXtalk.txt> = output of genNeighborXtalk app, measured w/ pulse=le_only calibGain=Off
       <calibGain.txt>     = calibGain ratios in txt file
       <inlCgOff.xml>      = intNonlin XML calibration file w/ valid calibGain OFF he curves
       <mevPerDAC.xml>     = Cal offline mevPerDAC calibration file
       <asym.xml>          = Cal offline asymmetry calibraiton file
       
"""

__facility__    = "Offline"
__abstract__    = "Convert neighbor xtalk curves from (src=LECIDAC_CGON, dest=HEX8ADC) to (src=HEDAC_SCINT_CGOFF, dest=HEDAC_SCINT_CGOFF)"
__author__      = "Z.Fewtrell"
__date__        = "$Date: 2007/08/17 16:35:28 $"
__version__     = "$Revision: 1.3 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"

### IMPORTS ###
import sys
import logging
import getopt
import array
import math

import ROOT
import csv

import calConstant
import cgc_util
import calCalibXML

# setup logger
logging.basicConfig()
log = logging.getLogger('charVal')
log.setLevel(logging.INFO)

### COMMANDLINE ARGS ####
# 1st check for optional args
try:
    opts, args = getopt.getopt(sys.argv[1:], "")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

# now check for req'd params
if len(args) != 5:
    log.error("bad n args: " + str(len(args)) + " " + __doc__)
    sys.exit(1)

# get filenames
(xtalkTXTPath, calibGainTXTPath, inlHEPath, mpdXMLPath, asymXMLPath) = args

# open and read XML intNonlin file
log.info("Reading cgoff inl file %s"%inlHEPath)
inlFile = calCalibXML.calIntNonlinCalibXML(inlHEPath)
inlData = inlFile.read()
inlTowers = inlFile.getTowers()
inlFile.close()
(adc2dac, dac2adc) = cgc_util.build_inl_splines(inlData, inlTowers)

# open and read XML MevPerDAC file
log.info("Opening input mevPerDAC XML file " + mpdXMLPath)
mpdFile = calCalibXML.calMevPerDacCalibXML(mpdXMLPath)
mpdData = mpdFile.read()
mpdTowers = mpdFile.getTowers()
mpdFile.close()

if mpdTowers != inlTowers:
    log.error("MPD towers %s && INL towers %s mismatch!"%(mpdTowers, inlTowers))
    sys.exit(-1)

# open and read XML Asymmetry file
log.info("Reading asym file %s"%asymXMLPath)
asymFile = calCalibXML.calAsymCalibXML(asymXMLPath)
asymTowers = asymFile.getTowers()
asymData = asymFile.read()
asymFile.close()
(pos2asym, asym2pos) = cgc_util.build_asym_splines(asymData, asymTowers)

if mpdTowers != asymTowers:
    log.error("MPD towers %s && ASYM towers %s mismatch!"%(mpdTowers, asymTowers))
    sys.exit(-1)

# open and read calibGainRatio txt file
log.info("Reading calibGain TXT file: " +  calibGainTXTPath)
(calibGainData, cgTowers) = cgc_util.read_perFace_txt(calibGainTXTPath)

if mpdTowers != cgTowers:
    log.error("MPD towers %s && CALIBGAIN towers %s mismatch!"%(mpdTowers, cgTowers))
    sys.exit(-1)

### open input 'csv' file ###
log.info("Opening input neighborXtalk txt file "+xtalkTXTPath)
csv_sample = file(xtalkTXTPath).read(8192)
dialect = csv.Sniffer().sniff(csv_sample)
has_header = csv.Sniffer().has_header(csv_sample)
infile = csv.reader(open(xtalkTXTPath,"r"),dialect = dialect)

# skip optional header in first line
if has_header : infile.next()

print ";dest_chan src_chan src_he_dac dest_he_dac"

### MAIN INPUT LOOP ###
for row in infile:
    destIdx = int(row[0])
    srcIdx  = int(row[1])

    # initial units=sourceing xtal, LEDAC, CGOFF
    srcLEDACCgOff = float(row[2])
    # initial units=destination xtal ADC
    destADC      = float(row[3])

    ### set up all needed indices ###
    (srcTwr, srcLyr, srcCol, srcFace, srcDiode) = cgc_util.diodeIdx2tuple(srcIdx)
    (destTwr, destLyr, destCol, destFace, destDiode) = cgc_util.diodeIdx2tuple(destIdx)

    destRow = calCalibXML.layerToRow(destLyr)
    srcRow = calCalibXML.layerToRow(srcLyr)

    destOnlineFace = calConstant.offline_face_to_online[destFace]
    srcOnlineFace  = calConstant.offline_face_to_online[srcFace]

    # assume source xtal is in le dac for now
    if srcDiode != calConstant.CDIODE_LRG:
        log.warning("Expecting source channel to be LE diode")
        continue


    ### DAC SCALE CONVERSION(S) ###
   
    # convert destination xtal ADC -> destination xtal CIDAC, CGOFF
    destRng = destDiode*2
    destDACCgOff = adc2dac[(destTwr, destRow, destOnlineFace, destCol, destRng)].Eval(destADC)

    # convert destination xtal CIDAC, CGOFF -> CGON
    destCalibGain = calibGainData[destTwr,destRow,destOnlineFace,destCol]
    
    if destDiode == 0: # le_diode
        destDACCgOn = destDACCgOff * destCalibGain
    else: #he diode
        destDACCgOn = destDACCgOff / destCalibGain

    # convert source xtal DACCGOFF -> DACCGON
    srcCalibGain = calibGainData[srcTwr,srcRow,srcOnlineFace,srcCol]
    if srcDiode == 0:
        srcLEDACCgOn = srcLEDACCgOff * srcCalibGain
    else:
        srcLEDACCgOn = srcLEDACCgOff / srcCalibGain


    # now convert source xtal ledac -> hedac
    src_le_mpd = mpdData[srcTwr, srcRow, srcCol, calConstant.CDIODE_LRG]
    src_he_mpd = mpdData[srcTwr, srcRow, srcCol, calConstant.CDIODE_SM]

    # find overall asymmetry (@ center of xtal)
    src_le_asymCtr = pos2asym[(srcTwr, srcRow, srcCol, calConstant.CDIODE_LRG)].Eval(0)
    src_he_asymCtr = pos2asym[(srcTwr, srcRow, srcCol, calConstant.CDIODE_SM)].Eval(0)

    if srcFace == calConstant.OFFLINE_FACE_POS:
        # desired ratio is PS_DAC / PL_DAC for calibGain On
        # mpdL = 1/sqrt(PL*NL)
        # mpdS = 1/sqrt(PS*NS)
        # asymL = log(PL/NL)
        # asymS = log(PS/NS)

        # sqrt((mpdL/mpdS)^2 * exp(asymS)/exp(asymL))
        # = sqrt((PS*NS)/(PL*NL) * (PS/NS)*(NL/PL))
        # = sqrt(PS*PS/(PL*PL))
        # = (mpdL/mpdS) * exp(asymS/2)/exp(asymL/2)

        src_lehe_ratio = (src_le_mpd/src_he_mpd) * math.exp(src_he_asymCtr/2)/math.exp(src_le_asymCtr/2)
    else: # negative xtal face
        # desired ratio is NS_DAC / NL_DAC for calibGain On
        # mpdL = 1/sqrt(PL*NL)
        # mpdS = 1/sqrt(PS*NS)
        # asymL = log(PL/NL)
        # asymS = log(PS/NS)

        # sqrt((mpdL/mpdS)^2 * exp(asymS)/exp(asymL))
        # = sqrt((PS*NS)/(PL*NL) * (NS/PS)*(PL/NL))
        # = sqrt(NS*NS/(NL*NL))
        # = (mpdL/mpdS) * exp(asymL/2)/exp(asymS/2)
        
        src_lehe_ratio = (src_le_mpd/src_he_mpd) * math.exp(src_le_asymCtr/2)/math.exp(src_he_asymCtr/2)
        
    srcHEDACCgOn = srcLEDACCgOn * src_lehe_ratio

    # change src index to reflect diode coversion
    srcDiode = 1
    srcIdx = cgc_util.tuple2diodeIdx((srcTwr, srcLyr, srcCol, srcFace, srcDiode))

    print destIdx, srcIdx, srcHEDACCgOn, destDACCgOn


