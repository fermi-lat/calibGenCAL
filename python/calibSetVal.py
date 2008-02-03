"""
Validate CAL offline calibration set as a suite (look @ inter-file-relationships)
line is:

calibSetVal [-V] [-L <log_file>] <ped.xml> <intNonlin.xml> <asym.xml> <mpd.xml> <tholdCI.xml>

where:
    -L <log_file>   - save console output to log text file
    -V              - verbose; turn on debug output
    <ped.xml>       - cal offline pedestal calibration xml file
    <intNonlin.xml> - cal offline pedestal calibration xml file
    <asym.xml>      - cal offline pedestal calibration xml file
    <mpd.xml>       - cal offline pedestal calibration xml file
    <tholdCI.xml>   - cal offline pedestal calibration xml file

"""


__facility__  = "Offline"
__abstract__  = "Validate CAL offline calibration set as a suite"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2007/08/17 16:35:28 $"
__version__   = "$Revision: 1.4 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import logging

import Numeric

import calCalibXML
import calConstant
import cgc_util

# return relative difference between 2 floats, avoid divide-by-zero error
def rel_diff(a,b):
    if (a != 0):
        return math.fabs((a-b)/a)
    elif (b !=0):
        return math.fabs((a-b)/b)
    return 0

nWarnings = 0
nErrors   = 0

# initial setup logger
logging.basicConfig()
log = logging.getLogger('calibSetVal')
log.setLevel(logging.INFO)


logName = None
# check command line
try:
    (opts,args) = getopt.getopt(sys.argv[1:], "-L:-V")
except getopt.GetoptError:
    log.error(__doc__)
    sys.exit(1)

for o in opts:
    if o[0] == '-L':
        logName = o[1]
    elif o[0] == '-V':
        log.setLevel(logging.DEBUG)

if len(args) != 5:
    log.error("Wrong number of arguments: %s"%__doc__)
    sys.exit(-1)

(pedPath, intNonlinPath, asymPath, mpdPath, tholdCIPath) = args


# logger config
if logName is None:
    logName = "calibSet.%s.val.log" % os.path.splitext(pedPath)[0]

if os.path.exists(logName):
    log.debug('Removing old log file %s', logName)
    os.remove(logName)    

hdl = logging.FileHandler(logName)
fmt = logging.Formatter('%(levelname)s %(message)s')
hdl.setFormatter(fmt)
log.addHandler(hdl)


# open and read XML Ped file
log.info("Reading ped file %s", pedPath)
xmlFile = calCalibXML.calPedCalibXML(pedPath)
pedData = xmlFile.read()
pedTowers = xmlFile.getTowers()
xmlFile.close()

# open and read XML IntNonlin file
log.info("Reading intNonlin file %s", intNonlinPath)
xmlFile = calCalibXML.calIntNonlinCalibXML(intNonlinPath)
intNonlinData = xmlFile.read()
intNonlinTowers = xmlFile.getTowers()
(inlLenData, inlDacData, inlAdcData) = intNonlinData
xmlFile.close()


### TEST 1: N_TOWERS MATCH ###
if (set(intNonlinTowers) != set(pedTowers)):
    log.error("intNonlin towers %s and ped towers %s mismatch!"%(intNonlinTowers, pedTowers))
    nErrors += 1
    sys.exit(-1)
    

# open and read XML Asym file
log.info("Reading asym file %s", asymPath)
xmlFile = calCalibXML.calAsymCalibXML(asymPath)
(xpos, asymData) = xmlFile.read()
asymTowers = xmlFile.getTowers()
xmlFile.close()


### TEST 1: N_TOWERS MATCH ###
if (set(asymTowers) != set(pedTowers)):
    log.error("asym towers %s and ped towers %s mismatch!"%(asymTowers, pedTowers))
    nErrors += 1
    sys.exit(-1)


# open and read XML MevPerDac file
log.info("Reading mevPerDac file %s", mpdPath)
xmlFile = calCalibXML.calMevPerDacCalibXML(mpdPath)
mpdData = xmlFile.read()
mpdTowers = xmlFile.getTowers()
xmlFile.close()

### TEST 1: N_TOWERS MATCH ###
if (set(mpdTowers) != set(pedTowers)):
    log.error("mevPerDac towers %s and ped towers %s mismatch!"%(mpdTowers, pedTowers))
    nErrors += 1
    sys.exit(-1)

# open and read XML Tholdci file
log.info("Reading tholdCI file %s", tholdCIPath)
xmlFile = calCalibXML.calTholdCICalibXML(tholdCIPath)
(tholdADCData, tholdULDData, tholdPedData) = xmlFile.read()
tholdCITowers = xmlFile.getTowers()
xmlFile.close()

### TEST 1: N_TOWERS MATCH ###
if (set(tholdCITowers) != set(pedTowers)):
    log.error("tholdCI towers %s and ped towers %s mismatch!"%(tholdCITowers, pedTowers))
    nErrors += 1
    sys.exit(-1)


for twr in pedTowers:
    for lyr in range(calConstant.NUM_LAYER):
        # calCalibXML uses 'row' indexing, not layer
        row = calCalibXML.layerToRow(lyr)
        for col in range(calConstant.NUM_FE):
            ### TEST 2 : All channels present ###
            mpdSm  = mpdData[twr,row,col,calConstant.CDIODE_SM]
            mpdLrg = mpdData[twr,row,col,calConstant.CDIODE_LRG]

            if mpdSm == 0 or mpdLrg == 0:
                log.error("MPD data missing: channel=%s"%[twr,lyr,col])
                nErrors += 1

            asymLL = asymData[twr,row,col,cgc_util.asymIdx[(calConstant.CDIODE_LRG,calConstant.CDIODE_LRG,False)]]
            asymLS = asymData[twr,row,col,cgc_util.asymIdx[(calConstant.CDIODE_LRG,calConstant.CDIODE_SM,False)]]
            asymSL = asymData[twr,row,col,cgc_util.asymIdx[(calConstant.CDIODE_SM,calConstant.CDIODE_LRG,False)]]
            asymSS = asymData[twr,row,col,cgc_util.asymIdx[(calConstant.CDIODE_SM,calConstant.CDIODE_SM,False)]]

            # TEST 3 : ASYM & MPD BIG/SM DIODE RATIOS #
            # posface_smalldiode_dac * negface_smalldiode_dac
            ps_x_ns = (1/mpdSm)**2
            # posface_largediode_dac * negface_largediode_dac
            pl_x_nl = (1/mpdLrg)**2
            
            # sm/lrg diode ratio from mevPerDAC info
            mpd_diode_ratio = ps_x_ns / pl_x_nl
            
            # get asymmetry values @ center of xtal
            pl_over_ns = (math.exp(asymLS[4])+math.exp(asymLS[5]))/2
            ps_over_nl = (math.exp(asymSL[4])+math.exp(asymSL[5]))/2
            
            
            # same sm/lrg diode ratio, this time from asym info
            asym_diode_ratio = ps_over_nl/pl_over_ns
            
            if rel_diff(asym_diode_ratio,mpd_diode_ratio) > .01:
                log.warning("MPD %s & Asym inter-diode ratio %s differ > 1 pct: channel=%s"%(mpd_diode_ratio, asym_diode_ratio, [twr,lyr,col]))

            if rel_diff(asym_diode_ratio,mpd_diode_ratio) > .05:
                log.warning("MPD %s & Asym inter-diode ratio %s differ > 5 pct: channel=%s"%(mpd_diode_ratio, asym_diode_ratio, [twr,lyr,col]))

            # max one asym value in a curve should be 0
            if len(Numeric.compress(asymLL==0,asymLL))>1 or \
               len(Numeric.compress(asymLS==0,asymLS))>1 or \
               len(Numeric.compress(asymSL==0,asymSL))>1 or \
               len(Numeric.compress(asymSS==0,asymSS))>1:
                log.error("Asym data missing: channel=%s"%[twr,lyr,col])
                nErrors +=1

            for face in range(calConstant.NUM_END):
                online_face = calConstant.offline_face_to_online[face]

                ### TEST 2 : All channels present ###
                lac = tholdADCData[twr][row][online_face][col][0]
                fle = tholdADCData[twr][row][online_face][col][1]
                fhe = tholdADCData[twr][row][online_face][col][2]

                if lac == 0 or fle == 0 or fhe == 0:
                    log.error("TholdCI data missing: channel=%s"%[twr,lyr,co,face])
                    nErrors += 1

                
                for rng in range(calConstant.NUM_RNG):
                    ### TEST 2 : All channels present ###
                    ped    = pedData[twr,row,online_face, col, rng, 0]
                    pedSig = pedData[twr,row,online_face, col, rng, 1]
                    if ped == 0 and pedSig == 0:
                        log.error("Ped data missing: channel=%s"%[twr,lyr,col,face,rng])
                        nErrors += 1

                    inlLen = inlLenData[rng][twr,row,online_face,col]
                    inlMax = inlAdcData[rng][twr,row,online_face,col,inlLen-1]
                    if inlLen <= 1 and inlMax == 0:
                        log.error("Inl data missing: channel=%s"%[twr,lyr,col,face,rng])
                        nErrors += 1

                    uld = tholdULDData[twr][row][online_face][col][rng]
                    if uld == 0:
                        log.error("TholdCI ULD data missing: channel=%s"%[twr,lyr,col,face,rng])
                        nErrors += 1

                    ### TEST 4: INL_MAX + PED <= 4095 ###
                    if inlMax + ped > 4095 + 1:
                        log.warning("INL MAX + PED %s > 4095: channel=%s"%(inlMax + ped, [twr,lyr,col,face,rng]))
                        nWarnings += 1

                    # TEST 5: ULD < INL ###
                    # otherwise simulation will never use next range
                    # uld check only valid for rng = LEX8 -> HEX1
                    if uld > inlMax and rng < 3: 
                        log.error("ULD %s > INL MAX %s: channel=%s"%(uld, inlMax, [twr,lyr,col,face,rng]))
                        nErrors += 1

                    # TEST 6: MUON_PED = THOLD_PED = INL_PED
                    muonPed = ped[0]
                    muonPedSig = pedSig[0]

                    tholdPed = tholdPedData[twr,row,online_face,col,rng][0]
                    inlPed   = 4095 - inlMax

                    pedList = [muonPed, inlPed, tholdPed]

                    pedDiff = max(pedList)-min(pedList)
                    if pedDiff > pedSig:
                        log.warning("Tholdci, muon, inl pedestals %s spread %s > muon ped sigma %s, channel %s"%(pedList, pedDiff, muonPedSig,[twr,lyr,col,face,rng]))
                        nWarnings +=1

                    if pedDiff > 2*pedSig:
                        log.error("Tholdci, muon, inl pedestals %s spread %s, > 2x muon ped sigma %s, channel %s"%(pedList, pedDiff, muonPedSig,[twr,lyr,col,face,rng]))
                        nErrors +=1

                    

                    

log.info("N_ERRORS=%s N_WARNINGS=%s"%(nWarnings, nErrors))

if nErrors == 0:
    log.info("TEST PASSED")
    sys.exit(0)
else:
    log.info("TEST FAILED")
    sys.exit(-1)

