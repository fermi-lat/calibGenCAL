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
__date__        = "$Date: 2006/03/29 22:01:32 $"
__version__     = "$Revision: 1.16 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import getopt
import logging
import ConfigParser

import Numeric

import calFitsXML
import calDacXML
import calConstant




if __name__ == '__main__':
    
    usage = "genFHEsettings [-V] <GeV> <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genFHEsettings')
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

    log.info("Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("Config file %s missing or empty", configName)
        sys.exit(1)

    # get input file names

    if 'infiles' not in sections:
        log.error("Config file %s missing [infiles] section", configName)
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
        log.error('Config file %s missing [infiles]:fhe2adc option', configName)
        sys.exit(1)
    if relName is None:
        log.error('Config file %s missing [infiles]:relgain option', configName)
        sys.exit(1)
    if adc2nrgName is None:
        log.error('Config file %s missing [infiles]:adc2nrg option', configName)
        sys.exit(1)
    if biasName is None:
        log.error('Config file %s missing [infiles]:bias option', configName)
        sys.exit(1)

    # get gain settings

    if 'gains' not in sections:
        log.error("Config file %s missing [gains] section", configName)
        sys.exit(1)

    heGain = None

    options = configFile.options('gains')
    for opt in options:
        if opt == 'hegain':
            heGain = int(configFile.get('gains', opt))

    if heGain is None:
        log.error('Config file %s missing [gains]:hegain option', configName)
        sys.exit(1)
    log.debug('Using HE gain %d', heGain)

    # get tower addresses

    if 'towers' not in sections:
        log.error("Config file %s missing [towers] section", configName)
        sys.exit(1)

    srcTwr = None
    destTwr = None

    options = configFile.options('towers')
    for opt in options:
        if opt == 'srctower':
            srcTwr = int(configFile.get('towers', 'srctower'))
            if srcTwr < 0 or srcTwr > 15:
                log.error('Option %s (%d) out of range', opt, srcTwr)
                sys.exit(1)
        if opt == 'desttower':
            destTwr = int(configFile.get('towers', 'desttower'))
            if destTwr < 0 or destTwr > 15:
                log.error('Option %s (%d) out of range', opt, destTwr)
                sys.exit(1)

    if srcTwr is None:
        log.error('Config file %s missing [towers]:srctower option', configName)
        sys.exit(1)
    log.debug('Using source tower %d', srcTwr) 
    if destTwr is None:
        log.error('Config file %s missing [towers]:desttower option', configName)
        sys.exit(1)
    log.debug('Using destination tower %d', destTwr)    

    # read FHE DAC ADC characterization file      

    log.info("Reading FHE ADC file %s", fheName)
    fio = calFitsXML.calFitsXML(fileName = fheName, mode = calFitsXML.MODE_READONLY)
    i = fio.info()
    if i['TTYPE1'] != 'fhe_dac':
        log.error("File %s is not an FHE ADC file.", fheName)
        sys.exit(1)
    if i['ERNG'] != 'HEX8':
        log.error("Only HEX8 energy range is supported for FHE DAC")
        sys.exit(1)
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, fheName)
        sys.exit(1)
    adcThresholds = fio.read()
    fio.close() 

    # read relative gain factor file     

    log.info("Reading relgain file %s", relName)
    fio = calFitsXML.calFitsXML(fileName = relName, mode = calFitsXML.MODE_READONLY)
    i = fio.info()
    if i['TTYPE1'] != 'relative gain factor':
        log.error("File %s is not a relgain ADC file", relName)
        sys.exit(1)
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, relName)
        sys.exit(1)
    relgain = fio.read()
    fio.close()

    # read ADC to energy conversion file
    
    log.info("Reading adc2nrg file %s", adc2nrgName)
    fio = calDacXML.calEnergyXML(adc2nrgName, 'adc2nrg')
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, adc2nrgName)
        sys.exit(1)
    adc2nrg = fio.read()
    fio.close()

    # read bias settings file
    
    log.info("Reading bias file %s", biasName)
    fio = calDacXML.calEnergyXML(biasName, 'thrBias')
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, biasName)
        sys.exit(1)
    biasTable = fio.read()
    fio.close()
      
    nrgIdx = calConstant.CRNG_HEX8
    nrgRangeMultiplier = 1.

    heGainIdx = max(0,heGain-8)
    if heGain == 0:
      heGainIdx = 8

    # split characterization data into fine and coarse DAC ranges

    fineThresholds = adcThresholds[srcTwr,:,:,:,0:64]
    log.debug('fineThresholds:[0,0,0,:]:%s', str(fineThresholds[0,0,0,:]))
    coarseThresholds = adcThresholds[srcTwr,:,:,:,64:]
    log.debug('coarseThresholds:[0,0,0,:]:%s', str(coarseThresholds[0,0,0,:]))    

    # calculate thresholds in ADC units from energy

    MeV = float(GeV) * 1000
    adcs = Numeric.ones((calConstant.NUM_ROW,calConstant.NUM_END,calConstant.NUM_FE),Numeric.Float) * MeV
    
    # relative gain factor

    adcs *= relgain[heGainIdx,nrgIdx,srcTwr,...] / relgain[8,nrgIdx,srcTwr,...]
    log.debug('adcs[0,0,0]:%6.3f relgain[%d,0,%d,0,0,0]:%6.3f relgain[8,0,%d,0,0,0]:%6.3f', \
                     adcs[0,0,0], heGainIdx, nrgIdx, relgain[heGainIdx,nrgIdx,srcTwr,0,0,0], nrgIdx, \
                     relgain[8,nrgIdx,srcTwr,0,0,0])
    
    # convert Mev to LEX8 ADC
    
    adcs /= adc2nrg[srcTwr,...,1]
    log.debug('adcs[0,0,0]:%6.3f adc2nrg[0,0,0,0,1]:%6.3f', adcs[0,0,0], adc2nrg[srcTwr,0,0,0,1])
    
    # convert to HEX1

    adcs /= nrgRangeMultiplier
    log.debug('adcs[0,0,0]:%6.3f nrgRangeMultiplier:%6.3f', adcs[0,0,0], nrgRangeMultiplier)
    
    # add bias correction

    adcs -= biasTable[srcTwr,...,1]
    log.debug('adcs[0,0,0]:%6.3f biasTable[0,0,0,0,1]:%6.3f', adcs[0,0,0], biasTable[srcTwr,0,0,0,1])

    # find setting that gives threshold
    # use fine DAC settings unless threshold is out of range
    # use coarse DAC settings for high thresholds

    nomSetting = Numeric.zeros((calConstant.NUM_TEM,calConstant.NUM_ROW,calConstant.NUM_END,calConstant.NUM_FE))
    q = Numeric.less(fineThresholds,adcs[...,Numeric.NewAxis])
    q1 = 64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)
    q1 = Numeric.choose(Numeric.equal(q1,64),(q1,0))
    nomSetting[destTwr,...] = q1
    q = Numeric.less(coarseThresholds,adcs[...,Numeric.NewAxis])
    q1 = (64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)) + 64
    q1 = Numeric.choose(Numeric.equal(q1,128),(q1,127))
    nomSetting = Numeric.choose(Numeric.equal(nomSetting,0),(nomSetting,q1))     

    # create output file

    log.info('Writing output file %s', outName)
    
    fio = calDacXML.calDacXML(outName, 'fhe_dac', calDacXML.MODE_CREATE)
    
    tlist = (destTwr,)
    outName = os.path.basename(outName)
    configName = os.path.basename(configName)
    fheName = os.path.basename(fheName)
    relName = os.path.basename(relName)
    adc2nrgName = os.path.basename(adc2nrgName)
    biasName = os.path.basename(biasName)
    
    fio.write(nomSetting, heGain = heGain, energy = MeV, filename = outName, cfgfilename = configName,
              adcfilename = fheName, relgainfilename = relName,
              engfilename = adc2nrgName, biasfilename = biasName, method = 'genFHEsettings:%s' % __release__,
              tems = tlist)
    
    fio.close()

    sys.exit(0)
    
    

   
