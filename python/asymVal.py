"""
Validate CAL Asym calibration data in XML format.  The command
line is:

asymVal [-R <root_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file

    <xml_file> The CAL Asym calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Asym calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/20 16:37:02 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant


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

                # create Big plots                     

                x = array.array('f')
                y = array.array('f')                        

                n = 0                           
                for xpos in xposData:
                    a = asymData[tem, row, fe, 0, n]
                    x.append(xpos)
                    y.append(a)
                    n += 1
                            
                g = ROOT.TGraph(len(x), x, y)

                g.SetMarkerStyle(5)
                g.SetMarkerColor(3)

                graph.append(g)
                leg.AddEntry(g, 'Big', 'P')

                # create Small plots                    

                x = array.array('f')
                y = array.array('f')                        

                n = 0                           
                for xpos in xposData:
                    a = asymData[tem, row, fe, 1, n]
                    x.append(xpos)
                    y.append(a)
                    n += 1
                            
                g = ROOT.TGraph(len(x), x, y)

                g.SetMarkerStyle(5)
                g.SetMarkerColor(2)

                graph.append(g)
                leg.AddEntry(g, 'Small', 'P')                    

                # create NsmallPbig plots                     

                x = array.array('f')
                y = array.array('f')                        

                n = 0                           
                for xpos in xposData:
                    a = asymData[tem, row, fe, 2, n]
                    x.append(xpos)
                    y.append(a)
                    n += 1
                            
                g = ROOT.TGraph(len(x), x, y)

                g.SetMarkerStyle(5)
                g.SetMarkerColor(4)

                graph.append(g)
                leg.AddEntry(g, 'NsmallPbig', 'P')

                # create PsmallNbig plots                     

                x = array.array('f')
                y = array.array('f')                        

                n = 0                           
                for xpos in xposData:
                    a = asymData[tem, row, fe, 3, n]
                    x.append(xpos)
                    y.append(a)
                    n += 1
                            
                g = ROOT.TGraph(len(x), x, y)

                g.SetMarkerStyle(5)
                g.SetMarkerColor(1)
                g.SetTitle('Asym_%s_Big_Small' % title)

                graph.append(g)
                leg.AddEntry(g, 'PsmallNbig', 'P')                

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


if __name__ == '__main__':

    usage = "usage: asymVal [-R <root_file>] <xml_file>"

    rootOutput = False

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]

    # open and read XML Ped file

    xmlFile = calCalibXML.calAsymCalibXML(xmlName)
    (xposData, asymData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write plots

        rootGraphs(xposData, asymData)

        # clean up

        rootFile.Close()
        
    sys.exit(0)