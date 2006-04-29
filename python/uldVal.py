"""
Validate CAL ULD settings XML files.  The command line is:

uldVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>]
       <cfg_file> <dac_xml_file>

where:
    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit
    -W <warn_limit> - warning limit
    -L <log_file>  - save console output to log text file
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <uld_xml_file>  = The ULD settings XML file to validate.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL ULD settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/03/16 18:04:40 $"
__version__     = "$Revision: 1.12 $, $Author: dwood $"
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




def rootHists(errData):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary"      
    hs = ROOT.TH1F(hName, '', 100, 0, (errLimit * 2))
    for e in errData:
        hs.Fill(e)
                    
    hs.Draw()
    cs.Update()
    cs.Write()



if __name__ == '__main__':

    usage = "uldVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] \
       <cfg_file> <dac_xml_file>"

    rootOutput = False
    warnLimit = 100.0
    errLimit = 150.0 
    valStatus = 0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('uldVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-E:-W:-L:-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        elif o[0] == '-E':
            errLimit = float(o[1])
        elif o[0] == '-W':
            warnLimit = float(o[1])
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('Deleting old log file %s', o[1])
                os.remove(o[1])
            hdl = logging.FileHandler(o[1])
            fmt = logging.Formatter('%(levelname)s %(message)s')
            hdl.setFormatter(fmt)
            log.addHandler(hdl)
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    dacName = args[1]
    
    log.debug('Using error limit %f', errLimit)
    log.debug('Using warning limit %f', warnLimit)

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
    i = fio.info()
    if i['TTYPE1'] != 'rng_uld_dac':
        log.error("File %s is not an ULD ADC file.", uldName)
        sys.exit(1)
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, uldName)
        sys.exit(1)
    adcData = fio.read()
    fio.close()

    # read DAC settings file

    log.info('Reading ULD settings file %s', uldName)
    fio = calDacXML.calDacXML(dacName, 'rng_uld_dac')
    twrs = fio.getTowers()
    if destTwr not in twrs:
        log.error("Dest twr %d data not found in file %s", destTwr, uldName)
        sys.exit(1)
    dacData = fio.read()
    fio.close()
    
    # validate data
    
    sat = [None, None, None]
    errData = []
    
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
            
                # get minimum saturation value
            
                for erng in range(3):
                    sat[erng] = adcData[erng, srcTwr, row, end, fe, -1]
                    
                satAdc = min(sat)
                for erng in range(3):
                    if satAdc == sat[erng]:
                        satRng = erng
                        break 
                    
                log.debug('Saturation %0.3f (%s) for %s%s,%d', satAdc, calConstant.CRNG[satRng], 
                    calConstant.CROW[row], calConstant.CPM[end], fe)
                                  
                # get ADC value for DAC setting
                    
                dac = int(dacData[destTwr, row, end, fe])
                adc = (adcData[satRng, srcTwr, row, end, fe, dac] + margin) 
                    
                # calculate error
                    
                err = abs(satAdc - adc)
                if err > warnLimit:
                    if err > errLimit:
                        log.error('err %0.2f > %0.2f for %s%s,%d', err, errLimit, calConstant.CROW[row],
                              calConstant.CPM[end], fe)
                        valStatus = 1
                    else:
                        log.warning('err %0.2f > %0.2f for %s%s,%d', err, warnLimit, calConstant.CROW[row],
                              calConstant.CPM[end], fe)
                errData.append(err)               
    
    
    # create ROOT output file
                              
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, dacName)
    sys.exit(valStatus)                                            
    

    