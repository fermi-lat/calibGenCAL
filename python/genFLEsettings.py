"""
Generate FLE Discriminator settings selected by Energy.  The command line is:

genFLEsettings [-V] <MeV> <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <MeV>           = The threshold energy in MeV units.
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The FLE settings XML file to output.
"""



__facility__    = "Offline"
__abstract__    = "Generate FLE Discriminator settings selected by Energy"
__author__      = "Byron Leas <leas@gamma.nrl.navy.mil>"
__date__        = "$Date: 2006/07/21 18:25:15 $"
__version__     = "$Revision: 1.18 $, $Author: dwood $"
__release__     = "$Name: v4r4 $"
__credits__     = "NRL code 7650"



import sys, os, time
import logging
import getopt
import ConfigParser

import Numeric

import calFitsXML
import calDacXML
import calConstant



if __name__ == '__main__':
    
    usage = "genFLEsettings [-V] <MeV> <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genFLEsettings')
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

    MeV = args[0]
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
        log.error("config file %s missing [infiles] section", configName)
        sys.exit(1) 

    fleName = None
    relName = None
    adc2nrgName = None
    biasName = None
    intNonlinName = None

    options = configFile.options('infiles')
    for opt in options:
        if opt == 'fle2adc':
            fleName = configFile.get('infiles', opt)
        elif opt == 'relgain':
            relName = configFile.get('infiles', opt)
        elif opt == 'adc2nrg':
            adc2nrgName = configFile.get('infiles', opt)
        elif opt == 'bias':
            biasName = configFile.get('infiles', opt)
        elif opt == 'intnonlin':
            intNonlinName = configFile.get('infiles', opt)
    if fleName is None:
        log.error('config file %s missing [infiles]:fle2adc option')
        sys.exit(1)
    if relName is None:
        log.error('config file %s missing [infiles]:relgain option')
        sys.exit(1)
    if adc2nrgName is None:
        log.error('config file %s missing [infiles]:adc2nrg option')
        sys.exit(1)
    if biasName is None:
        log.error('config file %s missing [infiles]:bias option')
        sys.exit(1)
    

    # get gain settings

    if 'gains' not in sections:
        log.error("config file %s missing [gains] section", configName)
        sys.exit(1)

    leGain = None

    options = configFile.options('gains')
    for opt in options:
        if opt == 'legain':
            leGain = int(configFile.get('gains', opt))

    if leGain is None:
        log.error('config file %s missing [gains]:legain option', configName)
        sys.exit(1)
    log.debug('using LE gain %d', leGain)

    # get tower addresses

    if 'towers' not in sections:
        log.error("config file %s missing [towers] section", configName)
        sys.exit(1)

    srcTwr = None
    destTwr = None

    options = configFile.options('towers')
    for opt in options:
        if opt == 'srctower':
            srcTwr = int(configFile.get('towers', 'srctower'))
            if srcTwr < 0 or srcTwr > 15:
                log.error('option %s (%d) out of range', opt, srcTwr)
                sys.exit(1)
        if opt == 'desttower':
            destTwr = int(configFile.get('towers', 'desttower'))
            if destTwr < 0 or destTwr > 15:
                log.error('option %s (%d) out of range', opt, destTwr)
                sys.exit(1)

    if srcTwr is None:
        log.error('config file %s missing [towers]:srctower option', configName)
        sys.exit(1)
    log.debug('using source tower %d', srcTwr) 
    if destTwr is None:
        log.error('config file %s missing [towers]:desttower option', configName)
        sys.exit(1)
    log.debug('using destination tower %d', destTwr)
    

    # read FLE DAC ADC characterization file
    
    log.info("Reading FLE ADC file %s", fleName)
    fio = calFitsXML.calFitsXML(fileName = fleName, mode = calFitsXML.MODE_READONLY)
    i = fio.info()
    if i['TTYPE1'] != 'fle_dac':
        log.error("File %s is not an FLE ADC file.", fheName)
        sys.exit(1)
    engRange = i['ERNG']
    if engRange != 'LEX8' and engRange != 'LEX1':
        log.error("Only LEX8 and LEX1 energy ranges supported for FLE DAC")
        sys.exit(1)
    log.debug('using energy range %s', engRange)	    
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, fleName)
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
    
    nrgRangeMultiplier = Numeric.ones((calConstant.NUM_ROW,calConstant.NUM_END,calConstant.NUM_FE),
          Numeric.Float)
	  
    if engRange == 'LEX8':
      nrgIdx = calConstant.CRNG_LEX8
    else:
      nrgIdx = calConstant.CRNG_LEX1
      nrgRangeMultiplier *= 9.0
            
    # split characterization data into fine and coarse DAC ranges

    fineThresholds = adcThresholds[srcTwr,:,:,:,0:64]
    log.debug('fineThresholds:[0,0,0,:]:%s', str(fineThresholds[0,0,0,:]))
    coarseThresholds = adcThresholds[srcTwr,:,:,:,64:]
    log.debug('coarseThresholds:[0,0,0,:]:%s', str(coarseThresholds[0,0,0,:]))

    # calculate thresholds in ADC units from energy    

    adcs = Numeric.ones((calConstant.NUM_ROW,calConstant.NUM_END,calConstant.NUM_FE),Numeric.Float) * float(MeV)
    log.debug('adcs[0,0,0]:%6.3f', adcs[0,0,0]) 
    
    # relative gain factor
    
    adcs *= relgain[leGain,nrgIdx,srcTwr,...]
    log.debug('adcs[0,0,0]:%6.3f relgain[%d,0,%d,0,0,0]:%6.3f', adcs[0,0,0], leGain, \
                     nrgIdx, relgain[leGain,nrgIdx,srcTwr,0,0,0])

    # energy->ADC conversion 
		     
    adcs /= adc2nrg[srcTwr,...,0]
    log.debug('adcs[0,0,0]:%6.3f adc2nrg[0,0,0,0]:%6.3f', adcs[0,0,0], adc2nrg[srcTwr,0,0,0,0])
    
    # trigger bias correction
    
    adcs -= biasTable[srcTwr,...,0]
    log.debug('adcs[0,0,0]:%6.3f biasTable[0,0,0,0]:%6.3f', adcs[0,0,0], biasTable[srcTwr,0,0,0,0])
    
    # convert to LEX1 ADC units
    
    adcs /= nrgRangeMultiplier
    log.debug('adcs[0,0,0]:%6.3f nrgRangeMultiplier[0,0,0]:%6.3f', adcs[0,0,0], nrgRangeMultiplier[0,0,0])
    
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
    dac = int(nomSetting[destTwr,0,0,0])
    log.debug("dac[0,0,0]:%d (%d)", dac, dac - 64)     

    # create output file

    log.info('Writing output file %s', outName)

    fio = calDacXML.calDacXML(outName, 'fle_dac', calDacXML.MODE_CREATE)
    
    tlist = (destTwr,)
    outName = os.path.basename(outName)
    configName = os.path.basename(configName)
    fleName = os.path.basename(fleName)
    relName = os.path.basename(relName)
    adc2nrgName = os.path.basename(adc2nrgName)
    biasName = os.path.basename(biasName)
    
    fio.write(nomSetting, leGain = leGain, energy = MeV, filename = outName, cfgfilename = configName,
              adcfilename = fleName, relgainfilename = relName,
              engfilename = adc2nrgName, biasfilename = biasName, method = 'genFLEsettings:%s' % __release__,
              tems = tlist)
    
    fio.close()

    sys.exit(0)
    
     

   
