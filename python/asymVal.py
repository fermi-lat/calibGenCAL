"""
Validate CAL Asym calibration data in XML format.  The command
line is:

asymVal [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for segment second derivative abs value
                    (default is 0.00005)
    -W <warn_limit> - warning limit for segment second derivative abs value
                    (default is 0.00010)
    <xml_file> The CAL Asym calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Asym calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/21 18:16:47 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, math
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant



VAL_NAMES = ('Big', 'Small', 'NsmallPbig', 'PsmallNbig')



def rootGraphs(xposData, asymData):

    # create ROOT plots XML IntNonlin data

    for tem in towers:
        for row in range(8):
            for fe in range(12):

                # create frame                        

                title = "T%d_%s_%d" % (tem, calConstant.CROW[row], fe)
                cName = "cg_%s" % title
                c = ROOT.TCanvas(cName, title, -1)

                leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)
                graph = []

                # create plots                 

                for val in range(4):                          

                    x = array.array('f')
                    y = array.array('f')                        

                    n = 0                           
                    for xpos in xposData:
                        a = asymData[tem, row, fe, val, n]
                        x.append(xpos)
                        y.append(a)
                        n += 1
                                
                    g = ROOT.TGraph(len(x), x, y)

                    g.SetMarkerStyle(5)
                    g.SetMarkerColor(val + 1)

                    graph.append(g)
                    leg.AddEntry(g, VAL_NAMES[val], 'P')                

                hist = g.GetHistogram()
                hist.SetMaximum(3)
                hist.SetMinimum(-3)

                # draw and write graphs

                hist.Draw()
                leg.Draw()
                c.Update()
                for g in graph:
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

    for val in range(4):

        hName = "h_Summary_%s" % VAL_NAMES[val]       
        hs = ROOT.TH1F(hName, 'Asym_Summary', 100, 0.0, errLimit)
        hs.SetLineColor(val + 1)
        hs.SetStats(False)
        sumHists[val] = hs
        sumLeg.AddEntry(hs, VAL_NAMES[val], 'L')
        cs.Update()
        
    # create ROOT histograms of sec deriv data per channel

    for tem in towers:
        for row in range(8):
            for fe in range(12):      

                title = "T%d_%s_%d" % (tem, calConstant.CROW[row], fe)
                cName = "ch_%s" % title
                    
                cx = ROOT.TCanvas(cName, title, -1)
                cx.SetGrid()
                cx.SetLogy()
                xtalLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
                hists = [None, None, None, None]
                    
                for val in range(4):
                        
                    hName = "h_%s_%s" % (title, VAL_NAMES[val])
                    hx = ROOT.TH1F(hName, 'Asym_%s' % title, 100, 0.0, errLimit)
                    hs = sumHists[val]
                    hx.SetLineColor(val + 1)
                    hx.SetStats(False)

                    eStr = errData[tem, row, fe, val]
                    err = eval(eStr)
                    for e in err:
                        hx.Fill(e)
                        hs.Fill(e)
                    hists[val] = hx
                    xtalLeg.AddEntry(hx, VAL_NAMES[val], 'L')
                    cx.Update()                       

                for val in range(4):
                    if val == 0:
                        dopt = ''
                    else:
                        dopt = 'SAME'
                    hx = hists[val]
                    hx.Draw(dopt)
                    cx.Update()

                xtalLeg.Draw()
                cx.Update()                    
                cx.Write()

    cs.cd()
    
    for val in range(4):

        hs = sumHists[val]
        if val == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()



def deriv2(p, a):

     y0 = (a[1] - a[0]) / (p[1] - p[0])
     y1 = (a[2] - a[1]) / (p[2] - p[1])
     x0 = (p[0] + p[1]) / 2
     x1 = (p[1] + p[2]) / 2
     z = (y1 - y0) / (x1 - x0)
     return math.fabs(z)



def calcError(xposData, asymData):

    errs = Numeric.zeros((16, 8, 12, 4), Numeric.PyObject)
    status = 0

    for tem in towers:
        for row in range(8):
            for fe in range(12):
                for val in range(4):

                    err = []
                    pLen = len(xposData) - 2
                        
                    for n in range(pLen):
                            
                        p = [0, 0, 0]
                        a = [0, 0, 0]
                        p[0] = xposData[n + 0]
                        p[1] = xposData[n + 1]
                        p[2] = xposData[n + 2]
                        a[0] = asymData[tem, row, fe, val, n + 0]
                        a[1] = asymData[tem, row, fe, val, n + 1]
                        a[2] = asymData[tem, row, fe, val, n + 2]

                        ex = deriv2(p, a)
                        err.append(ex)

                        if ex > warnLimit:
                            
                            if ex > errLimit:
                                msg = 'asymVal: %0.6f > %0.6f for %d,%s,%d,%s (%d,%0.3f)' % \
                                        (ex, errLimit, tem, calConstant.CROW[row], fe,
                                        VAL_NAMES[val], p[1], a[1])
                                log.error(msg)
                                status = 1
                            else:
                                msg = 'asymVal: %0.6f > %0.6f for %d,%s,%d,%s (%d,%0.3f)' % \
                                        (ex, warnLimit, tem, calConstant.CROW[row], fe,
                                        VAL_NAMES[val], p[1], a[1])
                                log.warning(msg)

                    errs[tem, row, fe, val] = repr(err)

    return (status, errs)



if __name__ == '__main__':

    usage = "usage: asymVal [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    errLimit = 0.00010
    warnLimit = 0.00005    

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

    log.debug('asymVal: using input file %s', xmlName)
    log.debug('asymVal: using sec deriv err limit %0.3f', errLimit)
    log.debug('asymVal: using sec deriv err limit %0.3f', warnLimit)

    # open and read XML Asym file

    xmlFile = calCalibXML.calAsymCalibXML(xmlName)
    (xposData, asymData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    (valStatus, errData) = calcError(xposData, asymData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write plots

        rootGraphs(xposData, asymData)

        # write error histograms

        rootHists(errData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('asymVal: validation %s for file %s', statusStr, xmlName)        
        
    sys.exit(valStatus)

    