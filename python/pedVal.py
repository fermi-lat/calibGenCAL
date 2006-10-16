"""
Validate CAL Ped calibration data in XML format.  The command
line is:

pedVal [-V] [-r] [-R <root_file>] [-L <log_file>] <xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL Ped calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Ped calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/09/26 22:13:01 $"
__version__   = "$Revision: 1.21 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import math
import getopt
import logging

import Numeric
import MLab

import calCalibXML
import calConstant



# limit values


pedWarnLimit = 850.0
pedErrLimit = 1000.0

x8ErrLimit = 10.0
x8WarnLimit = 6.5
    
x1ErrLimit = 2.0
x1WarnLimit = 1.0



def rootHists(errData, pedData, fileName):

    # create pedestal summary histogram
    
    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_Ped', 'Summary_Ped', -1)
    cs.cd()
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
    for erng in range(calConstant.NUM_RNG):
    
        hName = "h_Summary_Ped_%s" % calConstant.CRNG[erng]
        hs = ROOT.TH1F(hName, 'Ped_Summary: %s' % fileName, 100, 0.0, pedErrLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()  
        
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):             
                    for erng in range(calConstant.NUM_RNG):
                        hs = sumHists[erng]
                        ped = pedData[tem, row, end, fe, erng,0]
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
            axis = hs.GetXaxis()
            axis.SetTitle('Pedestal Value (ADC)')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
            
        hs.SetMaximum(hMax)    
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()                         

    # create summary x8 sig histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_x8', 'Summary_x8', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(0, 4, 2):

        hName = "h_Summary_x8_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'Sig_x8_Summary: %s' % fileName, 100, 0.0, x8ErrLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):             
                    for erng in range(0, 4, 2):
                        hs = sumHists[erng]
                        err = errData[tem, row, end, fe, erng]
                        hs.Fill(err)
                                             
    
    for erng in range(0, 4, 2):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Pedestal Sigma (ADC)')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()

    # create summary x1 sig histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_x1', 'Summary_x1', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(1, 4, 2):

        hName = "h_Summary_x1_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'Sig_x1_Summary: %s' % fileName, 100, 0.0, x1ErrLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):             
                    for erng in range(1, 4, 2):
                        hs = sumHists[erng]
                        err = errData[tem, row, end, fe, erng]
                        hs.Fill(err)
                                             
    
    for erng in range(1, 4, 2):

        hs = sumHists[erng]
        if erng == 1:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Pedestal Sigma (ADC)')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()



def calcError(pedData):

    status = 0

    # check pedestal average values
    
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(calConstant.NUM_RNG):
                    
                        ped = pedData[tem,row,end,fe,erng,0]
                        
                        if ped > pedWarnLimit:
                        
                            if ped > pedErrLimit:
                                msg = 'ped %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ped, pedErrLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.error(msg)
                                status = 1
                            else:         
                                msg = 'ped %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ped, pedWarnLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.warning(msg)
                            

    # check pedestal sigma values
    
    errs = pedData[...,1]

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(calConstant.NUM_RNG):

                                             
                        ex = errs[tem, row, end, fe, erng]
                                                   
                        if erng == 0 or erng == 2:
                            eLim = x8ErrLimit
                            wLim = x8WarnLimit
                        else:
                            eLim = x1ErrLimit
                            wLim = x1WarnLimit
                    
                        if ex > wLim:
                            
                            if ex > eLim:
                                msg = 'sigma %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ex, eLim, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.error(msg)
                                status = 1
                            else:
                                msg = 'sigma %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ex, wLim, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.warning(msg)

                        
    return (status, errs)




def average(data, tems):

    av = 0
    for t in tems:
        av += Numeric.average(data[t,...], axis = None)
    return (av / len(tems))
            


def stddev(data, tems):

    av = 0
    for t in tems:
        sd = MLab.std(Numeric.ravel(data[t,...]))
        av += (sd * sd)
        
    return math.sqrt(av / len(tems))



if __name__ == '__main__':

    usage = "pedVal [-V] [-r] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('pedVal')
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
    
    log.debug('Using sig err limit %0.3f for X8 ranges', x8ErrLimit)
    log.debug('Using sig err limit %0.3f for X1 ranges', x1ErrLimit)
    log.debug('Using sig warn limit %0.3f for X8 ranges', x8WarnLimit)
    log.debug('Using sig warn limit %0.3f for X1 ranges', x1WarnLimit)

    # open and read XML Ped file

    log.info("Reading file %s", xmlName)
    xmlFile = calCalibXML.calPedCalibXML(xmlName)
    pedData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()


    # validate calibration data

    (valStatus, errData) = calcError(pedData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData, pedData, xmlName)        

        # clean up

        rootFile.Close()
        
    # do simple stats 
    
    av = average(pedData[...,calConstant.CRNG_LEX8,0], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX8,0], towers)
    log.info("LEX8 pedestal values average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_LEX1,0], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX1,0], towers)
    log.info("LEX1 pedestal values average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX8,0], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX8,0], towers)
    log.info("HEX8 pedestal values average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX1,0], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX1,0], towers)
    log.info("HEX1 pedestal values average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_LEX8,1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX8,1], towers)
    log.info("LEX8 pedestal sigmas average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_LEX1,1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_LEX1,1], towers)
    log.info("LEX1 pedestal sigmas average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX8,1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX8,1], towers)
    log.info("HEX8 pedestal sigmas average=%f, stddev=%f", av, sd)
    
    av = average(pedData[...,calConstant.CRNG_HEX1,1], towers)
    sd = stddev(pedData[...,calConstant.CRNG_HEX1,1], towers)
    log.info("HEX1 pedestal sigmas average=%f, stddev=%f", av, sd)

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
