"""
Generate FHE Discriminator settings selected by Energy.  The command line is:

genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <GeV>           = The threshold energy in GeV units.
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The FHE settings XML file to output.
"""


__facility__    = "Offline"
__abstract__    = "Generate FHE Discriminator settings selected by Energy"
__author__      = "Byron Leas <leas@gamma.nrl.navy.mil>"
__date__        = "$Date: 2005/05/13 18:05:30 $"
__version__     = "$Revision: 1.2 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import getopt
import logging
import ConfigParser

import Numeric

import calFitsXML
import calDacXML
from calConstant import CXY, CPM



HEX8FLAG = True



if __name__ == '__main__':
    
    usage = "genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        
    args = opts[1]
    if len(args) != 3:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    outName = args[1]

    GeV = args[0]
    configName = args[1]
    outName = args[2]

    # read config file settings

    log.info("genFHEsettings: Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("genFHEsettings: config file %s missing or empty", configName)
        sys.exit(1)

    # get input file names

    if 'infiles' not in sections:
        log.error("genFHEsettings: config file %s missing [infiles] section", configName)
        sys.exit(1) 

    fheName = None
    relName = None
    adc2nrgName = None
    biasName = None
    
    options = configFile.options('infiles')
    for opt in options:
        if opt == 'fhe2adc':
            fheName = configFile.get('infiles', opt)
        elif opt == 'relgain':
            relName = configFile.get('infiles', opt)
        elif opt == 'adc2nrg':
            adc2nrgName = configFile.get('infiles', opt)
        elif opt == 'bias':
            biasName = configFile.get('infiles', opt)
    if fheName is None:
        log.error('genFHEsettings: config file %s missing [infiles]:fhe2adc option', configName)
        sys.exit(1)
    if relName is None:
        log.error('genFHEsettings: config file %s missing [infiles]:relgain option', configName)
        sys.exit(1)
    if adc2nrgName is None:
        log.error('genFHEsettings: config file %s missing [infiles]:adc2nrg option', configName)
        sys.exit(1)
    if biasName is None:
        log.error('genFHEsettings: config file %s missing [infiles]:bias option', configName)
        sys.exit(1)

    # get gain settings

    if 'gains' not in sections:
        log.error("genFHEsettings: config file %s missing [gains] section", configName)
        sys.exit(1)

    heGain = None

    options = configFile.options('gains')
    for opt in options:
        if opt == 'hegain':
            heGain = int(configFile.get('gains', opt))

    if heGain is None:
        log.error('genFHEsettings: config file %s missing [gains]:hegain option', configName)
        sys.exit(1)
    log.debug('genFHEsettings: using HE gain %d', heGain)

       # get tower addresses

    if 'towers' not in sections:
        log.error("genLACsettings: config file %s missing [towers] section", configName)
        sys.exit(1)

    srcTwr = None
    destTwr = None

    options = configFile.options('towers')
    for opt in options:
        if opt == 'srctower':
            srcTwr = int(configFile.get('towers', 'srctower'))
            if srcTwr < 0 or srcTwr > 15:
                log.error('genFHEsettings: option %s (%d) out of range', opt, srcTwr)
                sys.exit(1)
        if opt == 'desttower':
            destTwr = int(configFile.get('towers', 'desttower'))
            if destTwr < 0 or destTwr > 15:
                log.error('genFHEsettings: option %s (%d) out of range', opt, destTwr)
                sys.exit(1)

    if srcTwr is None:
        log.error('genFHEsettings: config file %s missing [towers]:srctower option', configName)
        sys.exit(1)
    log.debug('genFHEsettings: using source tower %d', srcTwr) 
    if destTwr is None:
        log.error('genFHEsettings: config file %s missing [towers]:desttower option', configName)
        sys.exit(1)
    log.debug('genFHEsettings: using destination tower %d', destTwr)    

    # read FHE DAC ADC characterization file      

    log.info("genFHEsettings: reading FHE ADC file %s", fheName)
    fio = calFitsXML.calFitsXML(fileName = fheName, mode = calFitsXML.MODE_READONLY)
    adcThresholds = fio.read()
    fio.close() 

    # read relative gain factor file     

    log.info("genFHEsettings: reading relgain file %s", relName)
    fio = calFitsXML.calFitsXML(fileName = relName, mode = calFitsXML.MODE_READONLY)
    relgain = fio.read()
    fio.close()

    # read ADC to energy conversion file
    
    log.info("genFHEsettings: reading adc2nrg file %s", adc2nrgName)
    fio = calDacXML.calEnergyXML(adc2nrgName, 'adc2nrg')
    adc2nrg = fio.read()
    fio.close()

    # read bias settings file
    
    log.info("genFHEsettings: reading bias file %s", biasName)
    fio = calDacXML.calEnergyXML(biasName, 'thrBias')
    biasTable = fio.read()
    fio.close()
      
    if HEX8FLAG:
        nrgIdx= 2
        nrgRangeMultiplier=1.
    else:
        nrgIdx=3
        nrgRangeMultiplier=9.

    heGainIdx = max(0,heGain-8)
    if heGain == 0:
      heGainIdx = 8

    fineThresholds = adcThresholds[srcTwr,:,:,:,0:64]
    log.debug('genFHEsettings: fineThresholds:[0,0,0,:]:%s', str(fineThresholds[0,0,0,:]))
    coarseThresholds = adcThresholds[srcTwr,:,:,:,64:]
    log.debug('genFHEsettings: coarseThresholds:[0,0,0,:]:%s', str(coarseThresholds[0,0,0,:]))

    adcs = Numeric.ones((8,2,12),Numeric.Float) * float(GeV) * 1000
    adcs = adcs * relgain[heGainIdx,nrgIdx,srcTwr,...] / relgain[8,nrgIdx,srcTwr,...]
    log.debug('genFHEsettings: adcs[0,0,0]:%6.3f relgain[%d,0,%d,0,0,0]:%6.3f relgain[8,0,%d,0,0,0]:%6.3f', \
                     adcs[0,0,0], heGainIdx, nrgIdx, relgain[heGainIdx,nrgIdx,srcTwr,0,0,0], nrgIdx, \
                     relgain[8,nrgIdx,srcTwr,0,0,0])
    adcs = adcs / adc2nrg[srcTwr,...,1]
    log.debug('genFHEsettings: adcs[0,0,0]:%6.3f adc2nrg[0,0,0,0,1]:%6.3f', adcs[0,0,0], adc2nrg[srcTwr,0,0,0,1])
    adcs = adcs / nrgRangeMultiplier
    log.debug('genFHEsettings: adcs[0,0,0]:%6.3f nrgRangeMultiplier:%6.3f', adcs[0,0,0], nrgRangeMultiplier)
    adcs = adcs - biasTable[srcTwr,...,1]
    log.debug('genFHEsettings: adcs[0,0,0]:%6.3f biasTable[0,0,0,0,1]:%6.3f', adcs[0,0,0], biasTable[srcTwr,0,0,0,1])

    nomSetting = Numeric.zeros((16,8,2,12))
    q = Numeric.choose(Numeric.less(fineThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = 64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)
    q1 = Numeric.choose(Numeric.equal(q1,64),(q1,0))
    nomSetting[destTwr,...] = q1
    q = Numeric.choose(Numeric.less(coarseThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = q1 = 64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)
    q1 = Numeric.choose(Numeric.equal(q1,128),(q1,127))
    nomSetting = Numeric.choose(Numeric.equal(nomSetting,0),(nomSetting,q1))     

    # create output file

    fio = calDacXML.calDacXML(outName, 'fle_dac', calDacXML.MODE_CREATE)
    tlist = (destTwr,)
    fio.write(nomSetting, hrefgain = heGain, tems = tlist)
    fio.close()

    sys.exit(0)
    
    

   