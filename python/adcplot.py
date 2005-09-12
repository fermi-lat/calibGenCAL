"""
Generate ROOT plots for CAL ADC/DAC characerization data.
"""


__facility__  = "Offline"
__abstract__  = "Generate ROOT plots for CAL ADC/DAC characerization data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/03/30 17:35:57 $"
__version__   = "$Revision: 1.1 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys
import logging
import array
import getopt

import Numeric

import ROOT

import calFitsXML
import calConstant




def plotDAC(data, info, twrs):
    """
    Create ROOT plots from ADC/DAC data
    """
    
    for tem in twrs:
        for layer in range(calConstant.NUM_LAYER):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    x = array.array('f')
                    y = array.array('f')
                    for dac in range(128):
                        x.append(dac)
                        y.append(data[tem, layer, end, fe, dac])
                        
                    title = "%s_T%d_%s%s_%d" % (info['TTYPE1'], tem, calConstant.CROW[layer],
                                                calConstant.CPM[end], fe)
                    cName = "c_%s" % title

                    c = ROOT.TCanvas(cName, title, -1)
                    g = ROOT.TGraph(128, x, y)

                    c.SetGridx()
                    c.SetGridy()
                            
                    g.SetMarkerStyle(5)
                    g.SetTitle(title)       
                    
                    g.Draw('AP')
                    c.Write()



def plotULD(data, info, twrs):
    """
    Create ROOT plots from ADC/ULD data
    """
    
    for tem in twrs:
        for layer in range(calConstant.NUM_LAYER):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                        
                    title = "%s_T%d_%s%s_%d" % (info['TTYPE1'], tem, calConstant.CROW[layer],
                                                calConstant.CPM[end], fe)
                    cName = "c_%s" % title

                    c = ROOT.TCanvas(cName, title, -1)
                    c.SetGrid()
                    graphs = []
                    leg = ROOT.TLegend(0.91, 0.50, 0.98, 0.60)

                    for erng in range(3):                    
                        
                        x = array.array('f')
                        y = array.array('f')
                        for dac in range(128):
                            x.append(dac)
                            y.append(data[erng, tem, layer, end, fe, dac])                    

                        g = ROOT.TGraph(128, x, y)
                        g.SetMarkerStyle(5)
                        g.SetMarkerColor(erng + 1)
                        g.SetTitle('%s_%s' % (title, calConstant.CRNG[erng]))

                        graphs.append(g)
                        leg.AddEntry(g, calConstant.CRNG[erng], 'P')

                    hist = g.GetHistogram()
                    hist.SetTitle(title)
                    hist.Draw()
                    leg.Draw()
                    c.Update()

                    for g in graphs:                    
                        g.Draw('P')
                        c.Update()
                        
                    c.Write()




if __name__ == '__main__':

    usage = "usage: adcplot [-V] <xml_file> <root_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('adcplot')
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

    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    xmlName = args[0]
    rootName = args[1]        

    # open and read XML/FITS characterization file

    xmlFile = calFitsXML.calFitsXML(fileName = xmlName, mode = calFitsXML.MODE_READONLY)
    data = xmlFile.read()
    info = xmlFile.info()
    twrs = xmlFile.getTowers()
    xmlFile.close()

    # create ROOT output file

    ROOT.gROOT.Reset()
    rootFile = ROOT.TFile(rootName, "recreate")

    # create ROOT plots

    type = info['TTYPE1']
    if type == 'fhe_dac' or type == 'fle_dac' or type == 'log_acpt':
        plotDAC(data, info, twrs)
    elif type == 'rng_uld_dac':
        plotULD(data, info, twrs)
    else:
        log.error("file type %s not supported", type)
        sys.exit(1)
    
    # clean up

    rootFile.Close()
    sys.exit(0)






