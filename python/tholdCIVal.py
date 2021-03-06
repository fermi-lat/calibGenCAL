"""
Validate CAL Thold_CI calibration data in XML format.  The command
line is:

tholdCIVal [-V] [-r] [-L <log_file>] [-R <root_file>] <xml_file>

where:
    -r              - generate ROOT output with default name
    -R <root_file>  - output validation diagnostics in ROOT file
    -L <log_file>   - save console output to log text file
    -V              - verbose; turn on debug output
    <xml_file>      - The CAL Thold_CI calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Thold_CI calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2008/02/15 22:47:14 $"
__version__   = "$Revision: 1.29 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os, math
import getopt
import logging

import numarray
import numarray.mlab

import calCalibXML
import calConstant
                  
                  

# validation limits


uldErrLimit     = 3000.0
uldWarnLimit    = 3200.0
    
                      
                  
                   

ADC_VALS = ('LAC', 'FLE', 'FHE', 'LEX8 ULD', 'HEX8 ULD')



def rootHists(adcData, uldData, pedData, fileName):

    # create summary ULD histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_ULD', 'Summary_ULD', -1)
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(calConstant.NUM_RNG-1): # ignore HEX1 ULD

        hName = "h_Summary_ULD_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'TholdCI_Summary_ULD: %s' % fileName, 100, uldErrLimit, 4095)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Threshold (ADC)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
    # create ROOT histograms of ULD data per tower

    for tem in towers:      

        title = "T%d" % (tem)
        cName = "ch_ULD_%s" % title
                        
        cx = ROOT.TCanvas(cName, title, -1)
        cx.SetGrid()
        cx.cd()
        xtalLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
        hists = [None, None, None, None]
                        
        for erng in range(calConstant.NUM_RNG-1): #ignore HEX1 ULD
                            
            hName = "h_%s_%d_%s" % (title, tem, calConstant.CRNG[erng])
            hx = ROOT.TH1F(hName, 'TholdCI_ULD_%s: %s' % (title, fileName), 100, uldErrLimit, 4095)
            hs = sumHists[erng]
            hx.SetLineColor(erng + 1)
            hx.SetStats(False)
            axis = hx.GetXaxis()
            axis.SetTitle('Threshold (ADC)')
            axis.CenterTitle()
            axis = hx.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
            
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                        uld = uldData[tem, row, end, fe, erng]
                        ped = pedData[tem, row, end, fe, erng]
                        hx.Fill(uld+ped)
                        hs.Fill(uld+ped)

            hists[erng] = hx
            xtalLeg.AddEntry(hx, calConstant.CRNG[erng], 'L')
            cx.Update()

        hMax = 0
        for erng in range(calConstant.NUM_RNG-1):
            hx = hists[erng]
            if hx.GetMaximum() > hMax:
                hMax = hx.GetMaximum()

        for erng in range(calConstant.NUM_RNG-1):
            if erng == 0:
                dopt = ''
            else:
                dopt = 'SAME'
            hx = hists[erng]
            hx.SetMaximum(hMax)
            hx.Draw(dopt)
            cx.Update()

        xtalLeg.Draw()
        cx.Update()                    
        cx.Write()

    cs.cd()

    hMax = 0
    for erng in range(calConstant.NUM_RNG-1):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for erng in range(calConstant.NUM_RNG-1):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()


    # create summary ADC histograms

    sumHists = [None, None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_ADC', 'Summary_ADC', -1)
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for val in range(5):

        hName = "h_Summary_ADC_%s" % ADC_VALS[val]       
        hs = ROOT.TH1F(hName, 'TholdCI_Summary_ADC: %s' % fileName, 100, 0, 4095)
        hs.SetLineColor(val + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Threshold (ADC)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[val] = hs
        sumLeg.AddEntry(hs, ADC_VALS[val], 'L')

    # create ROOT histograms of FLE/FHE/LAC data per tower        

    for tem in towers:      

        title = "T%d" % (tem)
        cName = "ch_ADC_%s" % title
                        
        cx = ROOT.TCanvas(cName, title, -1)
        cx.SetGrid()
        cx.cd()
        xtalLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
        hists = [None, None, None, None, None]
                        
        for val in range(5):
                            
            hName = "h_%s_%d_%s" % (title, tem, ADC_VALS[val])
            hx = ROOT.TH1F(hName, 'TholdCI_ADC_%s: %s' % (title, fileName), 100, 0, 4095)
            hs = sumHists[val]
            hx.SetLineColor(val + 1)
            hx.SetStats(False)
            axis = hx.GetXaxis()
            axis.SetTitle('Threshold (ADC)')
            axis.CenterTitle()
            axis = hx.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
            
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                    
                        if val == 3:
                            adc = uldData[tem, row, end, fe, calConstant.CRNG_LEX8]
                        elif val == 4:
                            adc = uldData[tem, row, end, fe, calConstant.CRNG_HEX8]
                        else:
                            adc = adcData[tem, row, end, fe, val]
                        hx.Fill(adc)
                        hs.Fill(adc)

            hists[val] = hx
            xtalLeg.AddEntry(hx, ADC_VALS[val], 'L')
            cx.Update()

        hMax = 0
        for val in range(5):
            hx = hists[val]
            if hx.GetMaximum() > hMax:
                hMax = hx.GetMaximum()

        for val in range(5):
            if val == 0:
                dopt = ''
            else:
                dopt = 'SAME'
            hx = hists[val]
            hx.SetMaximum(hMax)
            hx.Draw(dopt)
            cx.Update()

        xtalLeg.Draw()
        cx.Update()                    
        cx.Write()

    cs.cd()

    hMax = 0
    for val in range(5):
        hs = sumHists[val]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for val in range(5):

        hs = sumHists[val]
        if val == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()    

    # create summary pedestal value histograms
    
    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_Pedestal', 'Summary_Pedestal', -1)
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(calConstant.NUM_RNG):

        hName = "h_Summary_Pedestal_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'TholdCI_Summary_Pedestal: %s' % fileName, 100, 0, 1000)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Pedestal (ADC)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):             
                    for erng in range(calConstant.NUM_RNG):
                        hs = sumHists[erng]
                        ped = pedData[tem, row, end, fe, erng]
                        hs.Fill(ped)
                        
                        
    hMax = 0
    for erng in range(calConstant.NUM_RNG):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()                    
                        
    for erng in range(calConstant.NUM_RNG):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
            
        hs.SetMaximum(hMax)    
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()           
    
    

def calcError(adcData, uldData, pedData):

    status = 0

    # validate ULD values    

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(calConstant.NUM_RNG):

                            uld = uldData[tem, row, end, fe, erng]
                            ped = pedData[tem, row, end, fe, erng]

                            if erng == calConstant.CRNG_HEX1:
                                uldHighLimit = 4096.0
                            else:
                                uldHighLimit = 4095.0
                            
                            if (uld + ped) > uldHighLimit:
                                
                                log.error('ULD + PED %0.3f > %0.1f for T%d,%s%s,%d,%s',
                                          (uld + ped), uldHighLimit, tem, calConstant.CROW[row], calConstant.CPM[end],
                                          fe, calConstant.CRNG[erng])
                                status = 1

                            if uld < uldWarnLimit:
                                
                                if uld < uldErrLimit:
                                    msg = 'ULD %0.3f < %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldErrLimit, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.error(msg)
                                    status = 1
                                else:
                                    msg = 'ULD %0.3f < %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldWarnLimit, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.warning(msg)

                            # check ULD values against other thresholds

                            if erng == calConstant.CRNG_LEX8:

                                if adcData[tem, row, end, fe, 0] > uld:
                                    log.warning('LAC %0.3f > %0.3f for T%d,%s%s,%d,%s',
                                             adcData[tem, row, end, fe, 0], uld, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                              
                                
                            elif erng == calConstant.CRNG_HEX8:

                                if adcData[tem, row, end, fe, 2] > uld:                                
                                    log.warning('FHE %0.3f > %0.3f for T%d,%s%s,%d,%s',
                                             adcData[tem, row, end, fe, 2], uld, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                              

    return status



def average(data, tems):

    av = 0
    for t in tems:
        av += numarray.average(data[t,...], axis = None)
    return (av / len(tems))
            


def stddev(data, tems):

    av = 0
    for t in tems:
        sd = numarray.mlab.std(numarray.ravel(data[t,...]))
        av += (sd * sd)
        
    return math.sqrt(av / len(tems))



if __name__ == '__main__':


    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('tholdCIVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
    except getopt.GetoptError:
        log.error(__doc__)
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
        log.error(__doc__)
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

    log.debug('Using ULD lower err limit %6.3f', uldErrLimit)
    log.debug('Using ULD lower warn limit %6.3f', uldWarnLimit)

    # open and read XML Thold_CI file

    log.info('Reading file %s', xmlName) 
    xmlFile = calCalibXML.calTholdCICalibXML(xmlName)
    (adcData, uldData, pedData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(adcData, uldData, pedData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(adcData, uldData, pedData, xmlName)        

        # clean up

        rootFile.Close()
        
    # do simple stats 
    
    av = average(adcData[...,0], towers)
    sd = stddev(adcData[...,0], towers)
    log.info("LAC ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(adcData[...,1], towers)
    sd = stddev(adcData[...,1], towers)
    log.info("FLE ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(adcData[...,2], towers)
    sd = stddev(adcData[...,2], towers)
    log.info("FHE ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(uldData[...,calConstant.CRNG_LEX8], towers)
    sd = stddev(uldData[...,calConstant.CRNG_LEX8], towers)
    log.info("LEX8 ULD ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(uldData[...,calConstant.CRNG_LEX1], towers)
    sd = stddev(uldData[...,calConstant.CRNG_LEX1], towers)
    log.info("LEX1 ULD ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(uldData[...,calConstant.CRNG_HEX8], towers)
    sd = stddev(uldData[...,calConstant.CRNG_HEX8], towers)
    log.info("HEX8 ULD ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(uldData[...,calConstant.CRNG_HEX1], towers)
    sd = stddev(uldData[...,calConstant.CRNG_HEX1], towers)
    log.info("HEX1 ULD ADC threshold average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_LEX8], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX8], towers)
    log.info("LEX8 pedestal ADC value average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_LEX1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX1], towers)
    log.info("LEX1 pedestal ADC value average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX8], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX8], towers)
    log.info("HEX8 pedestal ADC value average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX1], towers)
    log.info("HEX1 pedestal ADC value average=%f, stddev=%f", av, sd)

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)        
        
    sys.exit(valStatus)

    
