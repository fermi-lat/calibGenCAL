"""
Validate CAL Thold_CI calibration data in XML format.  The command
line is:

tholdCIVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for segment second derivative abs value
                    (default is 0.00005)
    -W <warn_limit> - warning limit for segment second derivative abs value
                    (default is 0.00010)
    -L <log_file>   - save console output to log text file
    -V              - verbose; turn on debug output
    <xml_file> The CAL Asym calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Thold_CI calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/06/16 17:16:59 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os, math
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant
                   



def rootHists(adcData, uld):

    # create summary ULD histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(4):

        hName = "h_Summary_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'TholdCI_Summary', 100, uldErrLimits[0], uldErrLimits[1])
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
    # create ROOT histograms of ULD data per channel

    for tem in towers:      

        title = "T%d" % (tem)
        cName = "ch_%s" % title
                        
        cx = ROOT.TCanvas(cName, title, -1)
        cx.SetGrid()
        cx.cd()
        xtalLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
        hists = [None, None, None, None]
                        
        for erng in range(4):
                            
            hName = "h_%s_%d_%s" % (title, tem, calConstant.CRNG[erng])
            hx = ROOT.TH1F(hName, 'TholdCI_%s' % title, 100, uldErrLimits[0], uldErrLimits[1])
            hs = sumHists[erng]
            hx.SetLineColor(erng + 1)
            hx.SetStats(False)
            
            for row in range(8):
                for end in range(2):
                    for fe in range(12):
                        uld = uldData[tem, row, end, fe, erng]                        
                        hx.Fill(uld)
                        hs.Fill(uld)

            hists[erng] = hx
            xtalLeg.AddEntry(hx, calConstant.CRNG[erng], 'L')
            cx.Update()

        hMax = 0
        for erng in range(4):
            hx = hists[erng]
            if hx.GetMaximum() > hMax:
                hMax = hx.GetMaximum()

        for erng in range(4):
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
    for erng in range(4):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for erng in range(4):

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



def calcError(adcData, uldData):

    status = 0

    # validate ULD values    

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):
                    for erng in range(4):

                            uld = uldData[tem, row, end, fe, erng]
                            
                            if uld > uldWarnLimits[1]:
                                
                                if uld > uldErrLimits[1]:
                                    msg = 'tholdCIVal: %0.3f > %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldErrLimits[1], tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.error(msg)
                                    status = 1
                                else:
                                    msg = 'tholdCIVal: %0.3f > %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldWarnLimits[1], tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.warning(msg)

                            if uld < uldWarnLimits[0]:
                                
                                if uld < uldErrLimits[0]:
                                    msg = 'tholdCIVal: %0.3f < %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldErrLimits[0], tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.error(msg)
                                    status = 1
                                else:
                                    msg = 'tholdCIVal: %0.3f < %0.3f for T%d,%s%s,%d,%s' % \
                                            (uld, uldWarnLimits[0], tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.warning(msg)

                            # check ULD values against other thresholds

                            if erng == 0:

                                if adcData[tem, row, end, fe, 0] > uld:
                                    log.error('tholdCIVal: LAC %0.3f > LEX8 ULD %0.3f for T%d,%s%s,%d',
                                             adcData[tem, row, end, fe, 0], uld, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe)
                                    status = 1
                                              
                                if adcData[tem, row, end, fe, 1] > uld:
                                    log.error('tholdCIVal: FLE %0.3f > LEX8 ULD %0.3f for T%d,%s%s,%d',
                                             adcData[tem, row, end, fe, 1], uld, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe)
                                    status = 1

                            elif erng == 2:

                                if adcData[tem, row, end, fe, 2] > uld:                                
                                    log.error('tholdCIVal: FHE %0.3f > HEX8 ULD %0.3f for T%d,%s%s,%d',
                                             adcData[tem, row, end, fe, 2], uld, tem, calConstant.CROW[row],
                                             calConstant.CPM[end], fe)
                                    status = 1
                                                                
                                              

    return (status)



if __name__ == '__main__':

    usage = "usage: tholdCIVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    uldErrLimits = (3000.0, 4095.0)
    uldWarnLimits = (3200.0, 4000.0)    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
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
            errLimit = float(o[1])
        elif o[0] == '-W':
            warnLimit = float(o[1])
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('tholdCIVal: deleting old log file %s', o[1])
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

    log.debug('tholdCIVal: using input file %s', xmlName)
    log.debug('tholdCIVal: using ULD lower err limit %6.3f', uldErrLimits[0])
    log.debug('tholdCIVal: using ULD upper err limit %6.3f', uldErrLimits[1])
    log.debug('tholdCIVal: using ULD lower warn limit %6.3f', uldWarnLimits[0])
    log.debug('tholdCIVal: using ULD upper warn limit %6.3f', uldWarnLimits[1])

    # open and read XML Thold_CI file

    xmlFile = calCalibXML.calTholdCICalibXML(xmlName)
    (adcData, uldData, pedData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(adcData, uldData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(adcData, uldData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('tholdCIVal: validation %s for file %s', statusStr, xmlName)        
        
    sys.exit(valStatus)

    