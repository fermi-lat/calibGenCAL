"""
Plot CAL DacSlopes data as ROOT files:

dacSlopesPlot <xml_file> <root_file>

where:
    <xml_file>      - The CAL DacSlopes calibration XML file to plot. 
    <root_file>     - The name of the output ROOT file.   
"""


__facility__  = "Offline"
__abstract__  = "Plot CAL DacSlopes data as ROOT files"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/10/06 17:07:13 $"
__version__   = "$Revision: 1.9 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import logging
import array

import ROOT
import Numeric

import calCalibXML
import calConstant
                  
                  


DAC = array.array('f', Numeric.arange(0, 64, 1))



def linear(a, b, x):

    return a*x + b



def plotDAC(slopeData, offsetData, rangeData, dacType):

    
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
            
                # create frame 

                title = "%s_T%d_%s%s" % (dacType, tem, calConstant.CROW[row], calConstant.CPM[end])
                cName = "c_%s" % title
                
                c = ROOT.TCanvas(cName, title, -1)
                c.SetGridx()
                c.SetGridy()
                
                leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
                # generate idealized data for curves
    
                slope = slopeData[tem,row,end,:]  
                offset = offsetData[tem,row,end,:]
                rng = rangeData[tem,row,end,:]
                
                # plot data for a row
                
                graphs = []
                
                for fe in range(calConstant.NUM_FE):
                
                    eng = array.array('f', linear(slope[fe], offset[fe], DAC))
                    
                    g = ROOT.TGraph(len(DAC), DAC, eng)
                    g.SetLineColor(fe + 1)
                    if rng[fe] == calConstant.CDAC_COARSE:
                        g.SetLineStyle(2)
                    leg.AddEntry(g.GetHistogram(), "%d" % fe, 'L')
                    c.Update()
                    graphs.append(g)
                    
                    
                # set graph scales
                
                hMax = 0
                hMin = 0
                for g in graphs:
                    h = g.GetHistogram()    
                    if h.GetMaximum() > hMax:
                        hMax = h.GetMaximum()
                    if h.GetMinimum() < hMin:
                        hMin = h.GetMinimum() 
                        
                for g in graphs:
                    h = g.GetHistogram()
                    h.SetMaximum(hMax)
                    h.SetMinimum(hMin)           

                # display plots

                h = graphs[-1].GetHistogram()
                h.SetTitle("%s: %s" % (title, xmlName))
                axis = h.GetXaxis()
                axis.SetTitle('DAC')
                axis.CenterTitle()
                axis = h.GetYaxis()
                axis.SetTitle('Energy (MeV)')
                axis.CenterTitle()
                h.Draw()

                for g in graphs:
                    g.Draw('L')
                    c.Update()
                
                leg.Draw()
                c.Update()    
                c.Write()     
                    


if __name__ == '__main__':

    usage = "usage: dacSlopesPlot <xml_file> <root_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('dacSlopesPlot')
    log.setLevel(logging.INFO)

    # check command line

    if len(sys.argv) != 3:
        log.error(usage)
        sys.exit(1)    

    xmlName = sys.argv[1]
    rootName = sys.argv[2]
    

    # open and read XML DacSlopes file

    log.info('Reading file %s', xmlName) 
    xmlFile = calCalibXML.calDacSlopesCalibXML(xmlName)
    (dacData, uldData, rangeData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # create ROOT output file

    log.info('Creating file %s' % rootName)
    ROOT.gROOT.Reset()
    rootFile = ROOT.TFile(rootName, "recreate")
    
    # plot LAC curves
    
    plotDAC(dacData[...,0], dacData[...,1], rangeData[...,0], 'LAC') 
    
    # plot FLE curves
    
    plotDAC(dacData[...,2], dacData[...,3], rangeData[...,1], 'FLE')
    
    # plot FHE curves
    
    plotDAC(dacData[...,4], dacData[...,5], rangeData[...,2], 'FHE') 
    
    # clean up

    rootFile.Close()
    sys.exit(0)

    