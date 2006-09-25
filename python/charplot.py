"""
Generate ROOT plots for CAL ADC/DAC characerization data.  Include overlays of:
    - Raw characterization data
    - Filtered characterization data
    - Linear model fit

charplot [-V] <raw_xml_file> <filter_xml_file> <root_file>

Where:
    -V              = if present, run in verbose mode
    raw_xml_file    = raw (sparse) DAC/ADC characterization table XML data file
    filter_xml_file = filtered (smooth) DAC/ADC characterization table XML data file
    root_file       = ROOT file to store output graphs    
"""


__facility__  = "Offline"
__abstract__  = "Generate ROOT plots for CAL ADC/DAC characerization data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/07/05 18:35:32 $"
__version__   = "$Revision: 1.9 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys
import logging
import array
import getopt

import Numeric
import mpfit
import ROOT

import calFitsXML
import calConstant




def linear(a, b, x):

    return a*x + b



def residuals(p, y, x, fjac = None):

    err = y - linear(p[0], p[1], x)
    return (0, err)



def plotDAC(rawData, filterData, info, twrs, rawName, filterName):
    """
    Create ROOT plots from ADC/DAC data
    """

    pi = {'fixed' : 0, 'limited' : (0, 0), 'mpprint' : 0}
    pinfo = [pi, pi]
    x0 = Numeric.arange(0.0, 64.0, 1.0)
    
    for tem in twrs:
        for layer in range(calConstant.NUM_LAYER):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    # create frame 

                    title = "%s_T%d_%s%s_%d" % (info['TTYPE1'], tem, calConstant.CROW[layer],
                                                calConstant.CPM[end], fe)
                    cName = "c_%s" % title

                    c = ROOT.TCanvas(cName, title, -1)
                    c.SetGridx()
                    c.SetGridy()

                    lineGraph = []
                    markerGraph = []                    

                    # plot fine fit data

                    fineData = filterData[tem, layer, end, fe, 0:64]    
                    z = Numeric.nonzero(fineData)
                    yn = Numeric.take(fineData, z)
                    xn = Numeric.take(x0, z)
                    p0 = (20.0, -200.0)
                    fkw = {'x': xn, 'y' : yn}

                    fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                    if fit.status <= 0:
                        log.warning('mpfit error - %s', fit.errmsg)
                    else:
                        a = fit.params[0]
                        b = fit.params[1]
                        x = array.array('f')
                        y = array.array('f')
                        for dac in range(0, 64):
                            adc = linear(a, b, dac)
                            if adc < 0.0:
                                continue
                            x.append(dac)
                            y.append(adc)

                        g = ROOT.TGraph(len(x), x, y)
    
                        g.SetLineColor(4)
                        g.SetLineStyle(2)
                        g.SetLineWidth(2)
                        lineGraph.append(g)

                    # plot coarse fit data

                    coarseData = filterData[tem, layer, end, fe, 64:128]    
                    z = Numeric.nonzero(coarseData)
                    yn = Numeric.take(coarseData, z)
                    xn = Numeric.take(x0, z)
                    p0 = (20.0, -200.0)
                    fkw = {'x': xn, 'y' : yn}

                    fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                    if fit.status <= 0:
                        log.warning('mpfit error - %s', fit.errmsg)
                    else:
                        a = fit.params[0]
                        b = fit.params[1]
                        x = array.array('f')
                        y = array.array('f')
                        for dac in range(0, 64):
                            adc = linear(a, b, dac)
                            if adc < 0.0:
                                continue
                            x.append(dac + 64)
                            y.append(adc)

                        g = ROOT.TGraph(len(x), x, y)

                        g.SetLineColor(4)
                        g.SetLineStyle(2)
                        g.SetLineWidth(2)
                        lineGraph.append(g)                    

                    # plot raw data
                    
                    x = array.array('f')
                    y = array.array('f')
                    for dac in range(0, 128):
                        x.append(dac)
                        y.append(rawData[tem, layer, end, fe, dac])
                        
                    g = ROOT.TGraph(128, x, y)

                    g.SetMarkerColor(1)                    
                    g.SetMarkerStyle(4)
                    g.SetTitle(title)       
                    markerGraph.append(g)

                    # plot filtered data
                    
                    x = array.array('f')
                    y = array.array('f')
                    for dac in range(0, 128):
                        x.append(dac)
                        y.append(filterData[tem, layer, end, fe, dac])
                        
                    g = ROOT.TGraph(128, x, y)

                    g.SetMarkerColor(2)                            
                    g.SetMarkerStyle(5)
                    g.SetTitle(title)       
                    markerGraph.append(g)                     

                    # display plots

                    h = markerGraph[-1].GetHistogram()
                    h.SetTitle("%s: %s, %s" % (title, rawName, filterName))
                    axis = h.GetXaxis()
                    axis.SetTitle('DAC')
                    axis.CenterTitle()
                    axis = h.GetYaxis()
                    axis.SetTitle('ADC')
                    axis.CenterTitle()
                    h.Draw()

                    for g in lineGraph:
                        g.Draw('L')
                        c.Update
                    
                    for g in markerGraph:                    
                        g.Draw('P')
                        c.Update()
                        
                    c.Write()



def plotULD(rawData, filterData, info, twrs, rawName, filterName):
    """
    Create ROOT plots from ADC/ULD data
    """

    pi = {'fixed' : 0, 'limited' : (0, 0), 'mpprint' : 0}
    pinfo = [pi, pi]
    x0 = Numeric.arange(0.0, 64.0, 1.0)
    
    for tem in twrs:
        for layer in range(calConstant.NUM_LAYER):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    # create frame 

                    title = "%s_T%d_%s%s_%d" % (info['TTYPE1'], tem, calConstant.CROW[layer],
                                                calConstant.CPM[end], fe)
                    cName = "c_%s" % title

                    c = ROOT.TCanvas(cName, title, -1)
                    c.SetGridx()
                    c.SetGridy()

                    leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)                    

                    lineGraph = []
                    markerGraph = []
                    gMax = []

                    for erng in range(3):                    

                        # plot coarse fit data

                        coarseData = filterData[erng, tem, layer, end, fe, 64:128]
                        sat = coarseData[-1]
                        z = Numeric.nonzero(coarseData)
                        yn = Numeric.take(coarseData, z)
                        xn = Numeric.take(x0, z)
                        s = Numeric.less(yn, sat)
                        yn = Numeric.compress(s, yn)
                        xn = Numeric.compress(s, xn)
        
                        p0 = (40.0, -400.0)
                        fkw = {'x': xn, 'y' : yn}

                        fit = mpfit.mpfit(residuals, p0, functkw = fkw, parinfo = pinfo, quiet = 1)
                        if fit.status <= 0:
                            log.warning('mpfit error - %s', fit.errmsg)
                        else:
                            a = fit.params[0]
                            b = fit.params[1]
                            x = array.array('f')
                            y = array.array('f')
                            for dac in range(0, 64):
                                adc = linear(a, b, dac)
                                if adc < 0.0:
                                    continue
                                x.append(dac + 64)
                                y.append(adc)

                            g = ROOT.TGraph(len(x), x, y)

                            g.SetLineColor(4)
                            g.SetLineStyle(2)
                            g.SetLineWidth(2)
                            lineGraph.append(g)                        

                        # plot raw data
                    
                        x = array.array('f')
                        y = array.array('f')
                        for dac in range(0, 128):
                            x.append(dac)
                            y.append(rawData[erng, tem, layer, end, fe, dac])
                        
                        g = ROOT.TGraph(128, x, y)

                        g.SetMarkerColor(1)                    
                        g.SetMarkerStyle(4)
                        g.SetTitle(title)       
                        markerGraph.append(g)

                        # plot filtered data

                        gMax.append(max(filterData[erng, tem, layer, end, fe, :]))
                    
                        x = array.array('f')
                        y = array.array('f')
                        for dac in range(0, 128):
                            x.append(dac)
                            y.append(filterData[erng, tem, layer, end, fe, dac])
                        
                        g = ROOT.TGraph(128, x, y)

                        g.SetMarkerColor(erng + 1)                            
                        g.SetMarkerStyle(5)
                        g.SetTitle(title)       
                        markerGraph.append(g)

                        leg.AddEntry(g, calConstant.CRNG[erng], 'P')                        
                       

                    # display plots

                    h = markerGraph[-1].GetHistogram()
                    h.SetTitle("%s: %s, %s" % (title, rawName, filterName)) 
                    h.SetMaximum(max(gMax) + 200)
                    axis = h.GetXaxis()
                    axis.SetTitle('DAC')
                    axis.CenterTitle()
                    axis = h.GetYaxis()
                    axis.SetTitle('ADC')
                    axis.CenterTitle()
                    h.Draw()
                    leg.Draw()
                    c.Update()

                    for g in lineGraph:
                        g.Draw('L')
                        c.Update
                    
                    for g in markerGraph:                    
                        g.Draw('P')
                        c.Update()
                        
                    c.Write()                    




if __name__ == '__main__':

    usage = "usage: charplot [-V] <raw_xml_file> <filter_xml_file> <root_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('charplot')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        
    args = opts[1]    

    if len(args) != 3:
        log.error(usage)
        sys.exit(1)

    xmlNameRaw = args[0]
    xmlNameFilter = args[1]
    rootName = args[2]        

    # open and read XML/FITS raw characterization file

    log.info('Reading file %s' % xmlNameRaw)
    xmlFileRaw = calFitsXML.calFitsXML(fileName = xmlNameRaw, mode = calFitsXML.MODE_READONLY)
    rawData = xmlFileRaw.read()
    xmlFileRaw.close()

    # open and read XML/FITS filtered characterization file

    log.info('Reading file %s' % xmlNameFilter)
    xmlFileFilter = calFitsXML.calFitsXML(fileName = xmlNameFilter, mode = calFitsXML.MODE_READONLY)
    filterData = xmlFileFilter.read()
    filterInfo = xmlFileFilter.info()
    filterTwrs = xmlFileFilter.getTowers()
    xmlFileFilter.close()    

    # create ROOT output file

    log.info('Creating file %s' % rootName)
    ROOT.gROOT.Reset()
    rootFile = ROOT.TFile(rootName, "recreate")

    # create ROOT plots

    type = filterInfo['TTYPE1']
    if type == 'fhe_dac' or type == 'fle_dac' or type == 'log_acpt':
        plotDAC(rawData, filterData, filterInfo, filterTwrs, xmlNameRaw, xmlNameFilter)
    elif type == 'rng_uld_dac':
        plotULD(rawData, filterData, filterInfo, filterTwrs, xmlNameRaw, xmlNameFilter)
    else:
        log.error("file type %s not supported", type)
        sys.exit(1)
    
    # clean up

    rootFile.Close()
    sys.exit(0)






