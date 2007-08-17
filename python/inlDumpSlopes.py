"""
Fit straight line IntNonlin channel & dump y_intercept & slope to txt (also to ROOT)

output format is:
twr lyr col face range y_int slope

inlDumpSlopes [-d delim] <input_xml_file> [output_root_file]

where:
    <input_xml_file>   = input intNonlin GLAST Cal offline calibration file
    [output_root_file] = (optional) output root filename
    -d delim           = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Fit straight line IntNonlin channel & dump y_intercept & slope to txt"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/03/15 14:34:46 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import Numeric
import ROOT

import calCalibXML
import calConstant
import cgc_util
                  
if __name__ == '__main__':
    usage = "usage: python inlDumpSlopes.py [-d delim] <input_xml_file>"

    # check commandline
    delim = ' '
    try:
        (opts,args) = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)
    
    # opts has 2 parts, options (-abc ...) & remaining default params
    for o, a in opts:
        if o == '-d':
            delim = a

    if len(args) < 1:
        # should just be the one input file.
        print "no input file specified: ", usage
        sys.exit(1)

    # retrieve commandline parms
    inName  = args[0]

    rootFile = None
    if len(args) > 1:
        rootFile = ROOT.TFile(args[1],"RECREATE")

    # open and read XML intNonlin file

    xmlFile = calCalibXML.calIntNonlinCalibXML(inName)
    (lenData, dacData, adcData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out header as comment
    print ";twr lyr col face range y_intercept slope"

    # print out txt file.
    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for rng in range(calConstant.NUM_RNG):
                        length = int(lenData[rng][twr][row][online_face][col])
                        dacs = array.array('f',dacData[rng][twr][row][online_face][col])
                        adcs = array.array('f',adcData[rng][twr][row][online_face][col])

                        cname = "inlSlope"+" ".join([str(x) for x in rng,twr,lyr,col,face,rng])
                        c = ROOT.TCanvas(cname, cname, -1)
                        g = ROOT.TGraph(length, dacs, adcs)

                        c.SetGridx()
                        c.SetGridy()

                        g.Fit("pol1","Q")

                        g.SetMarkerStyle(5)
                        g.SetTitle(cname)
                        g.Draw('AP')
                        if rootFile is not None:
                            c.Write()

                        f = g.GetFunction("pol1")

                        y_int = f.GetParameter(0)
                        slope = f.GetParameter(1)
                        
                        print delim.join([str(x) for x in twr, lyr, col, face, rng, y_int, slope])

    # close out root file if we created one
    if rootFile is not None:
        rootFile.Write()
        rootFile.Close()

