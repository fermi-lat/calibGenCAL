"""
Generate LAC Discriminator settings selected by Energy.  The command line is:

genLACsettings [-V] <MeV> <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <MeV>           = The threshold energy in MeV units.
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The LAC settings XML file to output.
"""


__facility__    = "Offline"
__abstract__    = "Generate LAC Discriminator settings selected by Energy"
__author__      = "Byron Leas <leas@gamma.nrl.navy.mil>"
__date__        = "$Date: 2005/06/20 14:36:25 $"
__version__     = "$Revision: 1.9 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import logging
import getopt
import ConfigParser

import Numeric

import calFitsXML
import calDacXML
import calConstant


LEX8FLAG = True


if __name__ == '__main__':

    usage = "genLACsettings [-V] <MeV> <cfg_file> <out_xml_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genLACsettings')
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
        log.error("Config file %s missing [infiles] section", configName)
        sys.exit(1) 

    lacName = None
    relName = None
    adc2nrgName = None
    
    options = configFile.options('infiles')
    for opt in options:
        if opt == 'lac2adc':
            lacName = configFile.get('infiles', opt)
        elif opt == 'relgain':
            relName = configFile.get('infiles', opt)
        elif opt == 'adc2nrg':
            adc2nrgName = configFile.get('infiles', opt)   
    if lacName is None:
        log.error('Config file %s missing [infiles]:lac2adc option')
        sys.exit(1)
    if relName is None:
        log.error('Config file %s missing [infiles]:relgain option')
        sys.exit(1)
    if adc2nrgName is None:
        log.error('Config file %s missing [infiles]:adc2nrg option')
        sys.exit(1)
    

    # get gain settings

    if 'gains' not in sections:
        log.error("Config file %s missing [gains] section", configName)
        sys.exit(1)

    leGain = None

    options = configFile.options('gains')
    for opt in options:
        if opt == 'legain':
            leGain = int(configFile.get('gains', opt))

    if leGain is None:
        log.error('Config file %s missing [gains]:legain option', configName)
        sys.exit(1)
    log.debug('Using LE gain %d', leGain)

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
       

    # read LAC ADC characterization file

    log.info("Reading LAC ADC file %s", lacName)
    fio = calFitsXML.calFitsXML(fileName = lacName, mode = calFitsXML.MODE_READONLY)
    adcThresholds = fio.read()
    info = fio.info()
    fio.close()

    # read relative gain factor file     

    log.info("Reading relgain file %s", relName)
    fio = calFitsXML.calFitsXML(fileName = relName, mode = calFitsXML.MODE_READONLY)
    relgain = fio.read()
    fio.close()

    # read ADC to energy conversion file
    
    log.info("Reading adc2nrg file %s", adc2nrgName)
    fio = calDacXML.calEnergyXML(adc2nrgName, 'adc2nrg')
    adc2nrg = fio.read()
    fio.close()

    if LEX8FLAG:
      nrgIdx= calConstant.CRNG_LEX8
      nrgRangeMultiplier=1.
    else:
      nrgIdx=calConstant.CRNG_LEX1
      nrgRangeMultiplier=9.

    # split characterization data into fine and coarse DAC ranges      
    
    fineThresholds = adcThresholds[srcTwr,:,:,:,0:64]
    log.debug('fineThresholds:[0,0,0,0,:]:%s' % str(fineThresholds[0,0,0,:]))
    coarseThresholds = adcThresholds[srcTwr,:,:,:,64:]
    log.debug('coarseThresholds:[0,0,0,0,:]:%s' % str(coarseThresholds[0,0,0,:]))     

    # calculate thresholds in ADC units from energy

    MeV = float(MeV)
    if MeV < 2.0:

        # estimate for thresholds below 2 MeV

        offset = int((2.0 - MeV) * 3.0)
        MeV = 2.0
    else:
        offset = 0

    log.debug('Energy: %6.3f Offset:%6.3f', MeV, offset)   

    adcs = Numeric.ones((8,2,12), Numeric.Float32) * MeV
    adcs = adcs * relgain[leGain,nrgIdx,srcTwr,...]
    log.debug('adcs[0,0,0,0]:%6.3f relgain[%d,0,%d,0,0,0]:%6.3f' %(adcs[0,0,0], \
                    leGain,nrgIdx, relgain[leGain, nrgIdx,srcTwr,0,0,0]))
    adcs = adcs / adc2nrg[srcTwr,...,0]
    log.debug('adcs[0,0,0,0]:%6.3f adc2nrg[0,0,0,0]:%6.3f' %(adcs[0,0,0], adc2nrg[srcTwr,0,0,0,0]))
    adcs = adcs / nrgRangeMultiplier
    log.debug('adcs[0,0,0,0]:%6.3f nrgRangeMultiplier:%6.3f' %(adcs[0,0,0], nrgRangeMultiplier))

    # find setting that gives threshold
    # use fine DAC settings unless threshold is out of range
    # use coarse DAC settings for high thresholds

    nomSetting = Numeric.zeros((16,8,2,12))
    q = Numeric.choose(Numeric.less(fineThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = 64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)
    q1 = Numeric.choose(Numeric.equal(q1,64),(q1,0))
    nomSetting[destTwr,...] = q1
    q = Numeric.choose(Numeric.less(coarseThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = (64 - Numeric.argmax(q[:,:,:,::-1], axis = 3)) + 64
    q1 = Numeric.choose(Numeric.equal(q1,128),(q1,127))
    nomSetting = Numeric.choose(Numeric.equal(nomSetting,0),(nomSetting,q1))
    nomSetting = (nomSetting - offset)

    # create output file

    log.info('Writing output file %s', outName)
    tlist = (destTwr,)
    fio = calDacXML.calDacXML(outName, 'log_acpt', calDacXML.MODE_CREATE)
    fio.write(nomSetting, lrefgain = leGain, tems = tlist)
    fio.close()

    sys.exit(0)
    

