"""
Validate CAL IntNonlin calibration data in XML format.  The command
line is:

intNonlinVal [-V] [-r] [-L <log_file>] [-R <root_file>] <xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL Int_Nonlin calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL IntNonlin calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2007/03/20 19:23:47 $"
__version__   = "$Revision: 1.19 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os, math
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant
    



# validation limits

errLimit    = 2.0
warnLimit   = 1.5



def rootGraphs(lengthData, dacData, adcData, fileName):

    # create ROOT plots XML IntNonlin data

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    # create frame                        

                    title = "T%d_%s%s_%d" % (tem, calConstant.CROW[row], calConstant.CPM[end], fe)
                    cName = "cg_%s" % title
                    c = ROOT.TCanvas(cName, title, -1)
                    c.SetGrid()
                    c.Divide(1, 2)

                    x8Graph = []
                    x1Graph = []
                    x8Hist = None
                    x1Hist = None

                    # create X8 plots                        

                    c.cd(1)

                    x8Leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)                    
                        
                    for erng in range(0, 4, 2):                

                        adc = adcData[erng]
                        dac = dacData[erng]
                        length = lengthData[erng]
                            
                        x = array.array('f')
                        y = array.array('f')                        
                            
                        for n in range(length[tem, row, end, fe, 0]):
                            d = dac[tem, row, end, fe, n]
                            a = adc[tem, row, end, fe, n]
                            if a >= 0:
                                x.append(d)
                                y.append(a)
                            
                        g = ROOT.TGraph(len(x), x, y)
                        if erng < 2:
                            col = 2
                        else:
                            col = 4
                        g.SetMarkerStyle(5)
                        g.SetMarkerColor(col)
                        g.SetTitle('IntNonlin_%s_x8: %s' % (title, fileName))

                        x8Graph.append(g)
                        x8Leg.AddEntry(g, calConstant.CRNG[erng], 'P')
                        
                    x8Hist = g.GetHistogram()
                    axis = x8Hist.GetXaxis()
                    axis.SetTitle('CI DAC')
                    axis.CenterTitle()
                    axis = x8Hist.GetYaxis()
                    axis.SetTitle('ADC')
                    axis.CenterTitle()
                    
                    # create X1 plots                        

                    c.cd(2)
                    
                    x1Leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)
                        
                    for erng in range(1, 4, 2):
                        
                        adc = adcData[erng]
                        dac = dacData[erng]
                        length = lengthData[erng]
                            
                        x = array.array('f')
                        y = array.array('f')                        
                            
                        for n in range(length[tem, row, end, fe, 0]):
                            d = dac[tem, row, end, fe, n]
                            a = adc[tem, row, end, fe, n]
                            if a >= 0:
                                x.append(d)
                                y.append(a)
                            
                        g = ROOT.TGraph(len(x), x, y)
                        if erng < 2:
                            col = 2
                        else:
                            col = 4
                        g.SetMarkerStyle(5)
                        g.SetMarkerColor(col)
                        g.SetTitle('IntNonlin_%s_x1: %s' % (title, fileName))

                        x1Graph.append(g)
                        x1Leg.AddEntry(g, calConstant.CRNG[erng], 'P')

                    x1Hist = g.GetHistogram()
                    axis = x1Hist.GetXaxis()
                    axis.SetTitle('CI DAC')
                    axis.CenterTitle()
                    axis = x1Hist.GetYaxis()
                    axis.SetTitle('ADC')
                    axis.CenterTitle()

                    # draw and write graphs

                    c.cd(1)
                    x8Hist.Draw()
                    x8Leg.Draw()
                    c.Update()
                    for g in x8Graph:
                        g.Draw('P')
                        c.Update()    

                    c.cd(2)
                    x1Hist.Draw()
                    x1Leg.Draw()
                    c.Update()
                    for g in x1Graph:
                        g.Draw('P')
                        c.Update()

                    c.Write()



def rootHists(errData, fileName):

    # create summary sec deriv histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()
    cs.SetLogx()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(calConstant.NUM_RNG):

        hName = "h_Summary_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'IntNonlin_Summary: %s' % fileName, 100, 0.0, (errLimit + 1.0))
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
    # create ROOT histograms of sec deriv data per channel

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):      

                    title = "T%d_%s%s_%d" % (tem, calConstant.CROW[row], calConstant.CPM[end], fe)
                    cName = "ch_%s" % title
                    
                    cc = ROOT.TCanvas(cName, title, -1)
                    cc.SetGrid()
                    cc.SetLogy()
                    cc.SetLogx()
                    chanLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
                    hists = [None, None, None, None]
                    
                    for erng in range(calConstant.NUM_RNG):
                        
                        hName = "h_%s_%s" % (title, calConstant.CRNG[erng])
                        hc = ROOT.TH1F(hName, 'IntNonlin_%s: %s' % (title, fileName), 100, 0.0, (errLimit + 1.0))
                        hs = sumHists[erng]
                        hc.SetLineColor(erng + 1)
                        hc.SetStats(False)

                        eStr = errData[tem, row, end, fe, erng]
                        err = eval(eStr)
                        for e in err:
                            hc.Fill(e)
                            hs.Fill(e)
                        hists[erng] = hc
                        chanLeg.AddEntry(hc, calConstant.CRNG[erng], 'L')
                        cc.Update()                       

                    for erng in range(calConstant.NUM_RNG):
                        if erng == 0:
                            dopt = ''
                            hc = hists[0]
                            axis = hc.GetXaxis()
                            axis.SetTitle('Second Derivative')
                            axis.CenterTitle()
                            axis = hc.GetYaxis()
                            axis.SetTitle('Counts')
                            axis.CenterTitle()
                        else:
                            dopt = 'SAME'
                        hc = hists[erng]
                        hc.Draw(dopt)
                        cc.Update()

                    chanLeg.Draw()
                    cc.Update()                    
                    cc.Write()

    cs.cd()
    
    for erng in range(calConstant.NUM_RNG):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Second Derivative')
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



def deriv2(d, a):

     y0 = (a[1] - a[0]) / (d[1] - d[0])
     y1 = (a[2] - a[1]) / (d[2] - d[1])
     x0 = (d[0] + d[1]) / 2
     x1 = (d[1] + d[2]) / 2
     z = (y1 - y0) / (x1 - x0)
     return math.fabs(z)



def calcError(lengthData, dacData, adcData):

    errs = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE,
                          calConstant.NUM_RNG), Numeric.PyObject)
    status = 0

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(calConstant.NUM_RNG):

                        err = []
                        dac = dacData[erng]
                        adc = adcData[erng]
                        length = lengthData[erng]
                        dLen = length[tem, row, end, fe, 0] - 2
                        
                        for n in range(dLen):
                            
                            d = [0, 0, 0]
                            a = [0, 0, 0]
                            d[0] = dac[tem, row, end, fe, n + 0]
                            d[1] = dac[tem, row, end, fe, n + 1]
                            d[2] = dac[tem, row, end, fe, n + 2]
                            a[0] = adc[tem, row, end, fe, n + 0]
                            a[1] = adc[tem, row, end, fe, n + 1]
                            a[2] = adc[tem, row, end, fe, n + 2]

                            ex = deriv2(d, a)
                            err.append(ex)

                            if ex > warnLimit:
                            
                                if ex > errLimit:
                                    msg = '%0.3f > %0.3f for %d,%s%s,%d,%s (%d,%0.3f)' % \
                                            (ex, errLimit, tem, calConstant.CROW[row], calConstant.CPM[end], fe,
                                            calConstant.CRNG[erng], d[1], a[1])
                                    log.error(msg)
                                    status = 1
                                else:
                                    msg = '%0.3f > %0.3f for %d,%s%s,%d,%s (%d,%0.3f)' % \
                                            (ex, warnLimit, tem, calConstant.CROW[row], calConstant.CPM[end], fe,
                                            calConstant.CRNG[erng], d[1], a[1])
                                    log.warning(msg)

                        errs[tem, row, end, fe, erng] = repr(err)
                        
    return (status, errs)
                            
                                                    
                        


if __name__ == '__main__':


    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('intNonlinVal')
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

    log.debug('Using sec deriv err limit %0.3f', errLimit)
    log.debug('Using sec deriv warn limit %0.3f', warnLimit)    

    # open and read XML IntNonlin file

    log.info('Reading file %s', xmlName)
    xmlFile = calCalibXML.calIntNonlinCalibXML(xmlName)
    (lengthData, dacData, adcData) = xmlFile.read()
    info = xmlFile.info()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # calculate deviations from linearity

    (valStatus, errData) = calcError(lengthData, dacData, adcData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write plots

        rootGraphs(lengthData, dacData, adcData, xmlName)

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


    
