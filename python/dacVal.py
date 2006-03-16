"""
Validate CAL DAC settings XML files.  The command line is:

dacVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>]
       FLE|FHE|LAC <MeV> <cfg_file> <dac_xml_file>

where:
    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit
    -W <warn_limit> - warning limit
    -L <log_file>  - save console output to log text file
    -V              = verbose; turn on debug output
    FLE|FHE|LAC     = DAC type to validate
    <MeV>           = The threshold energy in MeV units.
    <cfg_file>      = The application configuration file to use.
    <dac_xml_file>  = The DAC settings XML file to validate.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/02/08 19:20:25 $"
__version__     = "$Revision: 1.1 $, $Author: dwood $"
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



def rootHists(engData):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary"      
    hs = ROOT.TH1F(hName, '', 100, MeV - (errLimit * 2), MeV + (errLimit * 2))
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                e = engData[row, end, fe]
                hs.Fill(e)
                    
    hs.Draw()
    cs.Update()
    cs.Write()



def engVal(errData):

    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):

                err = errData[row, end, fe]                
                if err > warnLimit:
                    if err > errLimit:
                        log.error('err %0.2f > %0.2f for %s%s,%d', err, errLimit, calConstant.CROW[row],
                                  calConstant.CPM[end], fe)
                        valStatus = 1
                    else:
                        log.warning('err %0.2f > %0.2f for %s%s,%d', err, warnLimit, calConstant.CROW[row],
                                    calConstant.CPM[end], fe)
                        


if __name__ == '__main__':
    
    usage = "dacVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] \
       FLE|FHE|LAC <MeV> <cfg_file> <dac_xml_file>"

    rootOutput = False
    valStatus = 0
    warnLimit = 5.0
    errLimit = 10.0    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('dacVal')
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
    if len(args) != 4:
        log.error(usage)
        sys.exit(1)

    dacType = args[0]
    MeV = float(args[1])
    configName = args[2]
    dacName = args[3]

    if dacType == 'FLE':
        type = 'fle_dac'
    elif dacType == 'FHE':
        type = 'fhe_dac'
        errLimit *= 8
        warnLimit *= 8
    elif dacType == 'LAC':
        type = 'log_acpt'
    else:
        log.error("DAC type %s not supported", dacType)
        sys.exit(1)

    log.debug('Using error limit %f', errLimit)
    log.debug('Using warning limit %f', warnLimit)
    log.debug('Using DAC name %s', type)
    log.debug('Using threshold %f MeV', MeV)

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

    charName = None
    relName = None
    adc2nrgName = None
    biasName = None

    if dacType == 'FLE':
        charOpt = 'fle2adc'
    elif dacType == 'FHE':
        charOpt = 'fhe2adc'
    elif dacType == 'LAC':
        charOpt = 'lac2adc'
    
    options = configFile.options('infiles')
    for opt in options:
        if opt == charOpt:
            charName = configFile.get('infiles', opt)
        elif opt == 'relgain':
            relName = configFile.get('infiles', opt)
        elif opt == 'adc2nrg':
            adc2nrgName = configFile.get('infiles', opt)
        elif opt == 'bias':
            biasName = configFile.get('infiles', opt)
    if charName is None:
        log.error('config file %s missing [infiles]:%s option', charOpt)
        sys.exit(1)
    if relName is None:
        log.error('config file %s missing [infiles]:relgain option')
        sys.exit(1)
    if adc2nrgName is None:
        log.error('config file %s missing [infiles]:adc2nrg option')
        sys.exit(1)
    if (dacType == 'FLE' or dacType == 'FHE') and biasName is None:
        log.error('config file %s missing [infiles]:bias option')
        sys.exit(1)
    

    # get gain settings

    if 'gains' not in sections:
        log.error("config file %s missing [gains] section", configName)
        sys.exit(1)

    gain = None

    if dacType == 'FHE':
        gainOpt = 'hegain'
    else:
        gainOpt = 'legain'
    
    options = configFile.options('gains')
    for opt in options:
        if opt == gainOpt:
            gain = int(configFile.get('gains', opt))            

    if gain is None:
        log.error('config file %s missing [gains]:%s option', configName, gainOpt)
        sys.exit(1)

    if dacType == 'FHE':
        gain = max(0, gain - 8)
        if gain == 0:
            gain = 8
        
    log.debug('using gain %d', gain)


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
    

    # read DAC/ADC characterization file
    
    log.info("reading %s ADC file %s", dacType, charName)
    fio = calFitsXML.calFitsXML(fileName = charName, mode = calFitsXML.MODE_READONLY)
    i = fio.info()
    if i['TTYPE1'] != type:
        log.error("File %s is not correct ADC file type", charName)
        sys.exit(1)
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, charName)
        sys.exit(1)
    adcThresholds = fio.read()
    fio.close() 

    # read relative gain factor file     

    log.info("reading relgain file %s", relName)
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
    
    log.info("reading adc2nrg file %s", adc2nrgName)
    fio = calDacXML.calEnergyXML(adc2nrgName, 'adc2nrg')
    twrs = fio.getTowers()
    if srcTwr not in twrs:
        log.error("Src twr %d data not found in file %s", srcTwr, adc2nrgName)
        sys.exit(1)
    adc2nrg = fio.read()
    fio.close()

    # read bias settings file      

    if dacType == 'FLE' or dacType == 'FHE':    
        log.info("reading bias file %s", biasName)
        fio = calDacXML.calEnergyXML(biasName, 'thrBias')
        twrs = fio.getTowers()
        if srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", srcTwr, biasName)
            sys.exit(1)
        biasTable = fio.read()
        fio.close()
    
    # read DAC settings file

    log.info('Reading DAC settings file %s', dacName)
    fio = calDacXML.calDacXML(dacName, type)
    twrs = fio.getTowers()
    if destTwr not in twrs:
        log.error("Dest twr %d data not found in file %s", destTwr, dacName)
        sys.exit(1)
    dacData = fio.read()
    fio.close()

    # lookup threshold ADC value from characterization table
    
    adcs = Numeric.zeros((calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE), Numeric.Float32)
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                d = int(dacData[destTwr, row, end, fe])
                a = adcThresholds[srcTwr, row, end, fe, d]
                adcs[row, end, fe] = a

    log.debug('adcs[0,0,:]:\n%s', adcs[0,0,:])               

    if dacType == 'FHE':
        idx = 1
    else:
        idx = 0
                
    # add bias correction

    if dacType == 'FLE' or dacType == 'FHE':  
        adcs = adcs + biasTable[srcTwr, ..., idx]
        log.debug('adcs[0,0,0]: %6.3f biasTable[0,0,0,0,%d]: %6.3f', adcs[0,0,0], idx, biasTable[srcTwr,0,0,0,idx])
    
    # convert to MeV
    
    eng = adcs * adc2nrg[srcTwr, ..., idx]
    log.debug('eng[0,0,0]: %6.3f adc2nrg[0,0,0,0,%d]: %6.3f', eng[0,0,0], idx, adc2nrg[srcTwr,0,0,0, idx]) 
    
    # adjust for gain

    if dacType == 'FHE':
        rng = calConstant.CRNG_HEX8
    else:
        rng = calConstant.CRNG_LEX8
        
    eng = eng / relgain[gain, rng, srcTwr, ...]
    if dacType == 'FHE':
        eng  = eng * relgain[8, rng, srcTwr, ...]
    log.debug('eng[0,0,0]: %6.3f relgain[%d,%d,0,0,0]: %6.3f ', eng[0,0,0], gain, rng, relgain[gain,rng,srcTwr,0,0,0]) 
    
    # calculate errors

    err = abs(eng - MeV)

    # do validation

    engVal(err)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(eng)        

        # clean up

        rootFile.Close()
        

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, dacName)
    sys.exit(valStatus)    

    sys.exit(0)
    

   