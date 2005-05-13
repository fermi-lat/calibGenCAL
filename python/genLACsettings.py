"""
Generate LAC Discriminator settings selected by Energy
"""


__facility__    = "Offline"
__abstract__    = "Generate LAC Discriminator settings selected by Energy"
__author__      = "Byron Leas <leas@gamma.nrl.navy.mil>"
__date__        = "$Date: 2005/04/01 18:08:35 $"
__version__     = "$Revision: 1.18 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import logging
import getopt
import ConfigParser

import Numeric

import calFitsXML
import calDacXML
from calConstant import CPM, CXY 


LEX8FLAG = True


if __name__ == '__main__':

    usage = "genLACsettings [-V] <MeV> <cfg_file> <out_xml_file>"

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

    MeV = args[0]
    configName = args[1]
    outName = args[2]

    # read config file settings

    log.info("genLACsettings: Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("genLACsettings: config file %s missing or empty", configName)
        sys.exit(1)

    # get input file names

    if 'infiles' not in sections:
        log.error("genLACsettings: config file %s missing [infiles] section", configName)
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
        log.error('genLACsettings: config file %s missing [infiles]:lac2adc option')
        sys.exit(1)
    if relName is None:
        log.error('genLACsettings: config file %s missing [infiles]:relgain option')
        sys.exit(1)
    if adc2nrgName is None:
        log.error('genLACsettings: config file %s missing [infiles]:adc2nrg option')
        sys.exit(1)
    

    # get gain settings

    if 'gains' not in sections:
        log.error("genLACsettings: config file %s missing [gains] section", configName)
        sys.exit(1)

    leGain = None

    options = configFile.options('gains')
    for opt in options:
        if opt == 'legain':
            leGain = int(configFile.get('gains', opt))

    if leGain is None:
        log.error('genLACsettings: config file %s missing [gains]:legain option', configName)
        sys.exit(1)
    log.debug('genLACsettings: using LE gain %d', leGain)
        

    # read LAC ADC characterization file

    log.info("genLACsettings: reading LAC ADC file %s", lacName)
    fio = calFitsXML.calFitsXML(fileName = lacName, mode = calFitsXML.MODE_READONLY)
    adcThresholds = fio.read()
    info = fio.info()
    fio.close()

    # read relative gain factor file     

    log.info("genLACsettings: reading relgain file %s", relName)
    fio = calFitsXML.calFitsXML(fileName = relName, mode = calFitsXML.MODE_READONLY)
    relgain = fio.read()
    fio.close()

    # read ADC to energy conversion file
    
    log.info("genLACsettings: reading adc2nrg file %s", adc2nrgName)
    fio = calDacXML.calEnergyXML(adc2nrgName, 'adc2nrg')
    adc2nrg = fio.read()
    fio.close()

    temId = 0
    if LEX8FLAG:
      nrgIdx= 0
      nrgRangeMultiplier=1.
    else:
      nrgIdx=1
      nrgRangeMultiplier=9.
    
    fineThresholds = adcThresholds[temId,:,:,:,0:64]
    log.debug('genLACsettings: fineThresholds:[0,0,0,0,:]:%s' % str(fineThresholds[0,0,0,:]))
    coarseThresholds = adcThresholds[temId,:,:,:,64:]
    log.debug('genLACsettings: coarseThresholds:[0,0,0,0,:]:%s' % str(coarseThresholds[0,0,0,:]))

    adcs = Numeric.ones((8,2,12), Numeric.Float32) * float(MeV)
    adcs = adcs * relgain[leGain,nrgIdx,temId,...]
    log.debug('genLACsettings: adcs[0,0,0,0]:%6.3f relgain[%d,0,%d,0,0,0]:%6.3f' %(adcs[0,0,0], \
                    leGain,nrgIdx, relgain[leGain, nrgIdx,temId,0,0,0]))
    adcs = adcs / adc2nrg[temId,...,0]
    log.debug('genLACsettings: adcs[0,0,0,0]:%6.3f adc2nrg[0,0,0,0]:%6.3f' %(adcs[0,0,0], adc2nrg[temId,0,0,0,0]))
    adcs = adcs / nrgRangeMultiplier
    log.debug('genLACsettings: adcs[0,0,0,0]:%6.3f nrgRangeMultiplier:%6.3f' %(adcs[0,0,0], nrgRangeMultiplier))

    nomSetting = Numeric.zeros((16,8,2,12))
    q = Numeric.choose(Numeric.less(fineThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = 64 - Numeric.argmax(q[temId,:,:,::-1], axis = 2)
    q1 = Numeric.choose(Numeric.equal(q1,64),(q1,0))
    nomSetting[temId,...] = q1
    q = Numeric.choose(Numeric.less(coarseThresholds,adcs[...,Numeric.NewAxis]),(0,1))
    q1 = q1 = 64 - Numeric.argmax(q[temId,:,:,::-1], axis = 2)
    q1 = Numeric.choose(Numeric.equal(q1,128),(q1,127))
    nomSetting = Numeric.choose(Numeric.equal(nomSetting,0),(nomSetting,q1))       

    # create output file

    log.info('genLACsettings: writing output file %s', outName)
    fio = calDacXML.calDacXML(outName, 'log_acpt', calDacXML.MODE_CREATE)
    fio.write(nomSetting, lrefgain = leGain)
    fio.close()

    sys.exit(0)
    

