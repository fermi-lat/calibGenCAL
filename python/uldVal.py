"""
Validate CAL ULD settings XML files.  The command line is:

uldVal [-V] [-r] [-R <root_file>] [-L <log_file>] <cfg_file> <dac_xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V              = verbose; turn on debug output
    <cfg_file>      = The application configuration file to use.
    <uld_xml_file>  = The ULD settings XML file to validate.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL ULD settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/10/16 15:46:32 $"
__version__     = "$Revision: 1.6 $, $Author: dwood $"
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




# validation limits


warnLimit   = 200.0
errLimit    = 400.0




def rootHists(errData, fileName):

    leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    hists = [None, None, None]

    # create summary histograms

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    for erng in range(3):
                            
        hName = "ULD_%s" % calConstant.CRNG[erng]
        hx = ROOT.TH1F(hName, 'ULD_Margin: %s' % fileName, 100, 0, 400)
        hx.SetLineColor(erng + 1)
        hx.SetStats(False)
        axis = hx.GetXaxis()
        axis.SetTitle('ULD Margin (ADC)')
        axis.CenterTitle()
        axis = hx.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()

        for err in errData[erng]:                                  
            hx.Fill(err)

        hists[erng] = hx
        leg.AddEntry(hx, calConstant.CRNG[erng], 'L')
        cs.Update()

    hMax = 0
    for erng in range(3):
        hx = hists[erng]
        if hx.GetMaximum() > hMax:
            hMax = hx.GetMaximum()

    for erng in range(3):
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hx = hists[erng]
        hx.SetMaximum(hMax)
        hx.Draw(dopt)
        cs.Update()

    leg.Draw()
    cs.Update()                    
    cs.Write()



if __name__ == '__main__':

    usage = "uldVal [-V] [-r] [-R <root_file>] [-L <log_file>] <cfg_file> <dac_xml_file>"

    rootOutput = False
    logName = None
     
    valStatus = 0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('uldVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
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
        elif o[0] == '-L':
            logName = o[1]
        elif o[0] == '-r':
            rootName = None
            rootOutput = True
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    dacName = args[1]
    
    ext = os.path.splitext(dacName)
    if rootOutput and rootName is None:
        rootName = "%s.val.root" % ext[0]
    if logName is None:
        logName = "%s.val.log" % ext[0]
        
    if os.path.exists(logName):
        log.debug('Removing old log file %s', logName)
        os.remove(logName)    

    hdl = logging.FileHandler(logName)
    fmt = logging.Formatter('%(levelname)s %(message)s')
    hdl.setFormatter(fmt)
    log.addHandler(hdl)
    
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
    
    errData = ([], [], [])
    
    for row in range(calConstant.NUM_ROW):
        for end in range(calConstant.NUM_END):
            for fe in range(calConstant.NUM_FE):
                for erng in range(3):
            
                    # get saturation value for range
            
                    sat = adcData[erng, srcTwr, row, end, fe, -1]
                    
                    log.debug('Saturation %0.3f for %s%s,%d,%s', sat, calConstant.CROW[row],
                              calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                  
                    # get ADC value for DAC setting
                    
                    dac = int(dacData[destTwr, row, end, fe])
                    adc = adcData[erng, srcTwr, row, end, fe, dac]
                    
                    # calculate error
                    
                    err = sat - adc
                    if err > warnLimit or err < margin:
                        if err > errLimit or err < margin:
                            log.error('err %0.2f > %0.2f for %s%s,%d,%s', err, errLimit, calConstant.CROW[row],
                                  calConstant.CPM[end], fe, calConstant.CRNG[erng])
                            valStatus = 1
                        else:
                            log.warning('err %0.2f > %0.2f for %s%s,%d,%s', err, warnLimit, calConstant.CROW[row],
                                  calConstant.CPM[end], fe, calConstant.CRNG[erng])
                            
                    errData[erng].append(err)               
    
    
    # create ROOT output file
                              
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData, uldName)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, dacName)
    sys.exit(valStatus)                                            
    

    