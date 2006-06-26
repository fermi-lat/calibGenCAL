"""
Validate CAL Ped calibration data in XML format.  The command
line is:

pedVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for pedestal sigma value for x8 ranges
                    (default is 10.0; x1 ranges use this value / 5)
    -W <warn_limit> - warning limit pedestal sigma value for x8 ranges
                     (default is 8.0; x1 ranges use this value / 5)
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL Ped calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Ped calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/06/26 18:06:15 $"
__version__   = "$Revision: 1.11 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calCalibXML
import calConstant




def rootHists(errData, pedData):

    # create pedestal summary histogram
    
    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_Ped', 'Summary_Ped', -1)
    cs.cd()
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
    for erng in range(calConstant.NUM_RNG):
    
        hName = "h_Summary_Ped_%s" % calConstant.CRNG[erng]
        hs = ROOT.TH1F(hName, 'Ped_Summary', 100, 0.0, 1000.0)
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
                        cs.Update() 
                        
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
        hs = ROOT.TH1F(hName, 'Ped_x8_Summary', 100, 0.0, x8ErrLimit)
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
                        cs.Update()                       
    
    for erng in range(0, 4, 2):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Pedestal Sigma')
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
        hs = ROOT.TH1F(hName, 'Ped_x1_Summary', 100, 0.0, x1ErrLimit)
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
                        cs.Update()                       
    
    for erng in range(1, 4, 2):

        hs = sumHists[erng]
        if erng == 1:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Pedestal Sigma')
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

    errs = pedData[...,1]
    status = 0

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
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ex, eLim, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.error(msg)
                                status = 1
                            else:
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (ex, wLim, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.warning(msg)

                        
    return (status, errs)



if __name__ == '__main__':

    usage = "pedVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    x8ErrLimit = 10.0
    x8WarnLimit = 7.0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('pedVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-E:-W:-L:-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        elif o[0] == '-E':
            x8ErrLimit = float(o[1])
        elif o[0] == '-W':
            x8WarnLimit = float(o[1])
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('Deleting old log file %s', o[1])
                os.remove(o[1])
            hdl = logging.FileHandler(o[1])
            fmt = logging.Formatter('%(levelname)s %(message)s')
            hdl.setFormatter(fmt)
            log.addHandler(hdl)
        elif o[0] == '-V':
            log.setLevel(logging.DEBUG)
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]
    x1ErrLimit = (x8ErrLimit / 5.0)
    x1WarnLimit = (x8WarnLimit / 5.0)

    log.debug('Using sig err limit %0.3f for x8 ranges', x8ErrLimit)
    log.debug('Using sig err limit %0.3f for x1 ranges', x1ErrLimit)
    log.debug('Using sig warn limit %0.3f for x8 ranges', x8WarnLimit)
    log.debug('Using sig warn limit %0.3f for x1 ranges', x1WarnLimit)

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

        rootHists(errData, pedData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
