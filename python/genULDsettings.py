"""
Generate ULD Discriminator settings selected by Energy.  The command line is:

genULDsettings [-V] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The ULD settings XML file to output.
"""



__facility__    = "Offline"
__abstract__    = "Generate ULD Discriminator settings selected by Energy"
__author__      = "D.L.Wood"
__date__        = "$Date: 2005/09/09 19:11:30 $"
__version__     = "$Revision: 1.10 $, $Author: dwood $"
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




if __name__ == '__main__':

    usage = "genULDsettings [-V] <cfg_file> <out_xml_file>"

    margin = 50    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genULDsettings')
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
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    outName = args[1]

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

    uldName = None
    
    options = configFile.options('infiles')
    for opt in options:
        if opt == 'uld2adc':
            uldName = configFile.get('infiles', opt)
    if uldName is None:
        log.error('Config file %s missing [infiles]:uld2adc option')
        sys.exit(1)
    

    # get margin settings

    if 'margins' not in sections:
        log.error("Config file %s missing [margins] section", configName)
        sys.exit(1)

    margin = None

    options = configFile.options('margins')
    for opt in options:
        if opt == 'adcmargin':
            margin = int(configFile.get('margins', opt))

    if margin is None:
        log.error('Config file %s missing [margins]:adcmargins option', configName)
        sys.exit(1)
    log.debug('Using ADC margin %d', margin)


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

    # read ULD/ADC characterization table

    log.info("Reading ULD ADC file %s", uldName)
    fio = calFitsXML.calFitsXML(fileName = uldName, mode = calFitsXML.MODE_READONLY)
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, uldName)
        sys.exit(1)
    adcData = fio.read()
    fio.close()

    # find saturation values    

    sat = [127, 127, 127]
    dacData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE),
                            Numeric.Int16)

    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                for erng in range(3):
                    adcmax = adcData[erng, srcTwr, row, end, fe, 127]
                    for dac in range(126, 64, -1):
                        adc = adcData[erng, srcTwr, row, end, fe, dac] + margin
                        if adc < adcmax:
                            break
                    sat[erng] = (dac - 1)

                dacData[destTwr, row, end, fe] = min(sat)                        
                        
    # write output file                    

    log.info('Writing ULD settings file %s', outName)
    
    fio = calDacXML.calDacXML(outName, 'rng_uld_dac', calDacXML.MODE_CREATE)
    
    tlist = (destTwr,)
    outName = os.path.basename(outName)
    configName = os.path.basename(configName)
    uldName = os.path.basename(uldName)
    
    fio.write(dacData, adcmargin = margin, filename = outName, cfgfilename = configName,
              adcfilename = uldName, method = 'genULDsettings:%s' % __release__, tems = tlist)

    fio.close()
    
    sys.exit(0)
    