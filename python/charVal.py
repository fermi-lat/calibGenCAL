"""
Validate CAL DAC/ADC characterization calibration data in XML format.  The command
line is:

charVal [-V] [-r] [-R <root_file>] [-L <log_file>] <xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL characterization XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL adc2nrg calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/09/25 23:10:26 $"
__version__   = "$Revision: 1.11 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric
import mpfit

import calFitsXML
import calConstant




def residuals(p, y, x, fjac = None):
  
    (a, b) = p
    err = y - a*x -b
    return (0, err)



def rootHists(errData, fileName):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hists = [None, None]
    leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for rng in range(2):

        hName = "h_Summary_%s" % calConstant.CDAC[rng]      
        hs = ROOT.TH1F(hName, 'DAC_Char: %s' % fileName, 100, 0.0, (dnormErrLimit * 2))
        hs.SetLineColor(rng + 1)
        hs.SetStats(False)
        for e in errData[rng]:
            hs.Fill(e)
                        
        hists[rng] = hs
        leg.AddEntry(hs, calConstant.CDAC[rng], 'L')
        cs.Update()

    hMax = 0
    for rng in range(2):
        hs = hists[rng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()

    for rng in range(2):
        if rng == 0:
            dopt = ''
            hs = hists[0]
            axis = hs.GetXaxis()
            axis.SetTitle('Fit Error')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
        hs = hists[rng]
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()
    
    leg.Draw()
    cs.Update()

    cs.Write()
    
    
    
def charVal(data):

    valStatus = 0
    
    pi = {'fixed' : 0, 'limited' : (0, 0), 'mpprint' : 0}
    pinfo = [pi, pi]
    x0 = Numeric.arange(0.0, 64.0, 1.0)
    
    
    # check for pedestal subtraction error
    
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    
                    adc = data[tem, row, end, fe, 0]
                    if adc > 100.0:
                        log.error('pedestal err for %s%s,%d,0 - %0.3f', calConstant.CROW[row], 
			                    calConstant.CPM[end], fe, adc)
                        valStatus = 1
                        
                    adc = data[tem, row, end, fe, 64]
                    if adc > 100.0:
                        log.error('pedestal err for %s%s,%d,64 - %0.3f', calConstant.CROW[row], 
			                    calConstant.CPM[end], fe, adc) 
                        valStatus = 1                           
                            
    
    # check for curve linearity
    
    errData = ([], [])    

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    fineData = data[tem, row, end, fe, 0:64]
                    coarseData = data[tem, row, end, fe, 64:128]                    

                    # fit FINE range data                    
                    
                    z = Numeric.nonzero(fineData)
                    y = Numeric.take(fineData, z)
                    x = Numeric.take(x0, z)
                    p0 = (20.0, -200.0)
                    fkw = {'x': x, 'y' : y}

                    if len(x) < 3:
                        log.error('Too little data: %d,%s%s,%d,FINE', tem, calConstant.CROW[row],
                                      calConstant.CPM[end], fe)
                        valStatus = 1
                    else:

                        fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                        if fit.status <= 0:
                            log.error('mpfit error - %s', fit.errmsg)
                            sys.exit(1)
                        dnorm = (fit.fnorm / len(x))
                        errData[0].append(dnorm)
                        log.debug("%d,%s%s,%d,FINE: %0.1f %0.1f %0.2f", tem, calConstant.CROW[row],
                                calConstant.CPM[end], fe, fit.params[0], fit.params[1], dnorm)

                        if dnorm > dnormWarnLimit:
                            if dnorm > dnormErrLimit:
                                log.error('dnorm %0.2f > %0.2f for %d,%s%s,%d,FINE', dnorm, dnormErrLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe)
                                valStatus = 1
                            else:
                                log.warning('dnorm %0.2f > %0.2f for %d,%s%s,%d,FINE', dnorm, dnormWarnLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe)

                    # fit coarse range data

                    z = Numeric.nonzero(coarseData)
                    y = Numeric.take(coarseData, z)
                    x = Numeric.take(x0, z)
                    p0 = (40, -400)
                    fkw = {'x': x, 'y' : y}
                    
                    if len(x) < 3:
                        log.error('Too little data: %d,%s%s,%d,COARSE', tem, calConstant.CROW[row],
                                      calConstant.CPM[end], fe)
                    else:

                        fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                        if fit.status <= 0:
                            log.error('mpfit error - %s', fit.errmsg)
                            sys.exit(1)
                        dnorm = (fit.fnorm / len(x))
                        errData[1].append(dnorm)
                        log.debug("%d,%s%s,%d,COARSE: %0.1f %0.1f %0.2f", tem, calConstant.CROW[row],
                                calConstant.CPM[end], fe, fit.params[0], fit.params[1], dnorm)

                        if dnorm > dnormWarnLimit:
                            if dnorm > dnormErrLimit:
                                log.error('dnorm %0.2f > %0.2f for %d,%s%s,%d,COARSE', dnorm, dnormErrLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe)
                                valStatus = 1
                            else:
                                log.warning('dnorm %0.2f > %0.2f for %d,%s%s,%d,COARSE', dnorm, dnormWarnLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe) 
    
    return (valStatus, errData)
    
    
    
def uldVal(data):

    valStatus = 0
    
    pi = {'fixed' : 0, 'limited' : (0, 0), 'mpprint' : 0}
    pinfo = [pi, pi]
    x0 = Numeric.arange(0.0, 64.0, 1.0)
    
    
    # check for curve linearity
    
    errData = ([], [])    

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(3): 

                        coarseData = data[erng, tem, row, end, fe, 64:128]
                                          
                        # fit coarse range data

                        sat = coarseData[-1]
                        z = Numeric.nonzero(coarseData)
                        y = Numeric.take(coarseData, z)
                        x = Numeric.take(x0, z)
                        s = Numeric.less(y, sat)
                        y = Numeric.compress(s, y)
                        x = Numeric.compress(s, x)
                        
                        p0 = (40, -400)
                        fkw = {'x': x, 'y' : y}
                    
                        if len(x) < 3:
                            log.error('Too little data: %d,%s%s,%d,%s,COARSE', tem, calConstant.CROW[row],
                                      calConstant.CPM[end], fe, calConstant.CRNG[erng])
                        else:

                            fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                            if fit.status <= 0:
                                log.error('mpfit error - %s', fit.errmsg)
                                sys.exit(1)
                            dnorm = (fit.fnorm / len(x))
                            errData[1].append(dnorm)
                            log.debug("%d,%s%s,%d,%s,COARSE: %0.1f %0.1f %0.2f", tem, calConstant.CROW[row],
                                calConstant.CPM[end], fe, calConstant.CRNG[erng], fit.params[0], fit.params[1], dnorm)

                            if dnorm > dnormWarnLimit:
                                if dnorm > dnormErrLimit:
                                    log.error('dnorm %0.2f > %0.2f for %d,%s%s,%d,%s,COARSE', dnorm, dnormErrLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    valStatus = 1
                                else:
                                    log.warning('dnorm %0.2f > %0.2f for %d,%s%s,%d,%s,COARSE', dnorm, dnormWarnLimit, tem,
                                          calConstant.CROW[row], calConstant.CPM[end], fe, calConstant.CRNG[erng]) 
    
    return (valStatus, errData)  
                                                 



if __name__ == '__main__':

    usage = "charVal [-V] [-r] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
    logName = None

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('charVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        elif o[0] == '-L':
            logName = o[1]
        elif o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-r':
            rootName = None
            rootOutput = True
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]
    ext = os.path.splitext(xmlName)
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

    # open and read XML adc2nrg file

    log.info('Reading file %s', xmlName)
    xmlFile = calFitsXML.calFitsXML(fileName = xmlName)
    data = xmlFile.read()
    towers = xmlFile.getTowers()
    info = xmlFile.info()
    type = info['TTYPE1']
    xmlFile.close()
 
    # determine validation limits based on DAC type
    
    if type == 'log_acpt':
        dnormWarnLimit = 200.0
        dnormErrLimit = 400.0
    elif type == 'fhe_dac':
        dnormWarnLimit = 500.0
        dnormErrLimit = 1000.0    
    else:
        dnormWarnLimit = 400.0
        dnormErrLimit = 800.0
                
    log.debug('Validating file type %s', type)
    log.debug('Using error limit %f', dnormErrLimit)
    log.debug('Using warning limit %f', dnormWarnLimit)
                            
    # do validation
    
    if type == 'rng_uld_dac':
        (valStatus, errData) = uldVal(data)
    else:    
        (valStatus, errData) = charVal(data)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData, xmlName)        

        # clean up

        rootFile.Close()
        

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
    
