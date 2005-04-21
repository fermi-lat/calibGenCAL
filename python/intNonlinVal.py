"""
Validate CAL IntNonlin calibration data in XML format.  The command
line is:

intNonlinVal [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>

where:

    -E <err_limit> - error limit for segment second derivative abs value
                    (default is 1.0)
    -W <warn_limit> - warning limit for segment second derivative abs value
                    (default is 2.0)
    -R <root_file> - output validation diagnostics in ROOT file

    <xml_file> The CAL Int_Nonlin calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL IntNonlin calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/20 16:37:02 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, math
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant



def rootGraphs(dacData, adcData):

    # create ROOT plots XML IntNonlin data

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):

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

                        n = 0
                        adc = adcData[erng]
                        dac = dacData[erng]
                            
                        x = array.array('f')
                        y = array.array('f')                        
                            
                        for d in dac:
                            a = adc[tem, row, end, fe, n]
                            if a >= 0:
                                x.append(d)
                                y.append(a)
                            n += 1
                            
                        g = ROOT.TGraph(len(x), x, y)
                        if erng < 2:
                            col = 2
                        else:
                            col = 4
                        g.SetMarkerStyle(5)
                        g.SetMarkerColor(col)
                        g.SetTitle('IntNonlin_%s_x8' % title)

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

                        n = 0
                        adc = adcData[erng]
                        dac = dacData[erng]
                            
                        x = array.array('f')
                        y = array.array('f')                        
                            
                        for d in dac:
                            a = adc[tem, row, end, fe, n]
                            if a >= 0:
                                x.append(d)
                                y.append(a)
                            n += 1
                            
                        g = ROOT.TGraph(len(x), x, y)
                        if erng < 2:
                            col = 2
                        else:
                            col = 4
                        g.SetMarkerStyle(5)
                        g.SetMarkerColor(col)
                        g.SetTitle('IntNonlin_%s_x1' % title)

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



def rootHists(errData):

    # create summary sec deriv histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(4):

        hName = "h_Summary_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'IntNonlin_Summary', 100, 0.0, errLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
    # create ROOT histograms of sec deriv data per channel

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):      

                    title = "T%d_%s%s_%d" % (tem, calConstant.CROW[row], calConstant.CPM[end], fe)
                    cName = "ch_%s" % title
                    
                    cc = ROOT.TCanvas(cName, title, -1)
                    cc.SetGrid()
                    cc.SetLogy()
                    chanLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
                    hists = [None, None, None, None]
                    
                    for erng in range(4):
                        
                        hName = "h_%s_%s" % (title, calConstant.CRNG[erng])
                        hc = ROOT.TH1F(hName, 'IntNonlin_%s' % title, 100, 0.0, errLimit)
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

                    for erng in range(4):
                        if erng == 0:
                            dopt = ''
                        else:
                            dopt = 'SAME'
                        hc = hists[erng]
                        hc.Draw(dopt)
                        cc.Update()

                    chanLeg.Draw()
                    cc.Update()                    
                    cc.Write()

    cs.cd()
    
    for erng in range(4):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
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
     d = (y1 - y0) / (x1 - x0)
     return math.fabs(d)



def calcError(dacData, adcData):

    errs = Numeric.zeros((16, 8, 2, 12, 4), Numeric.PyObject)

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):
                    for erng in range(4):

                        err = []
                        dac = dacData[erng]
                        adc = adcData[erng]
                        dLen = len(dac) - 2
                        
                        for n in range(dLen):
                            
                            d = [0, 0, 0]
                            a = [0, 0, 0]
                            d[0] = dac[n + 0]
                            d[1] = dac[n + 1]
                            d[2] = dac[n + 2]
                            a[0] = adc[tem, row, end, fe, n + 0]
                            a[1] = adc[tem, row, end, fe, n + 1]
                            a[2] = adc[tem, row, end, fe, n + 2]
                            if a[0] < 0 or a[1] < 0 or a[2] < 0:
                                continue

                            ex = deriv2(d, a)
                            err.append(ex)

                            if ex > warnLimit:
                            
                                if ex > errLimit:
                                    msg = 'intNonlinVal: %0.3f > %0.3f for %d,%s%s,%d,%s (%d,%0.3f)' % \
                                            (ex, errLimit, tem, calConstant.CROW[row], calConstant.CPM[end], fe,
                                            calConstant.CRNG[erng], d[1], a[1])
                                    log.error(msg)
                                else:
                                    msg = 'intNonlinVal: %0.3f > %0.3f for %d,%s%s,%d,%s (%d,%0.3f)' % \
                                            (ex, warnLimit, tem, calConstant.CROW[row], calConstant.CPM[end], fe,
                                            calConstant.CRNG[erng], d[1], a[1])
                                    log.warning(msg)

                        errs[tem, row, end, fe, erng] = repr(err)

    return errs                        
                            
                                                    
                        


if __name__ == '__main__':

    usage = "usage: intNonlinVal [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    errLimit = 2.0
    warnLimit = 1.0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-E:-W:")
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
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]

    # open and read XML IntNonlin file

    xmlFile = calCalibXML.calIntNonlinCalibXML(xmlName)
    (dacData, adcData) = xmlFile.read()
    info = xmlFile.info()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # calculate deviations from linearity

    errData = calcError(dacData, adcData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write plots

        rootGraphs(dacData, adcData)

        # write error histograms

        rootHists(errData)        
                              
        # clean up

        rootFile.Close()

    
    sys.exit(0)


    
