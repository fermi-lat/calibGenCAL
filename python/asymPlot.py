"""
Create plots of Cal asymmetry curves

asymXML2TXT <input_xml_file> <output_root_file

where:
    <input_xml_file> = input asymmetry GLAST Cal offline calibration file
    <output_root_file> = output ROOT file with plots

"""


__facility__  = "Offline"
__abstract__  = "Create plots of Cal asymmetry curves"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/09/13 18:31:45 $"
__version__   = "$Revision: 1.11 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import getopt
import sys
import calCalibXML
import ROOT
import cgc_util
import array
import Numeric

if __name__ == '__main__':
    usage = "usage: python asymXML2TXT.py <input_xml_file> <output_root_file>"

    # check commandline
    try:
        (opts,args) = getopt.getopt(sys.argv[1:], "")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    # opts has 2 parts, options (-abc ...) & remaining default params
    #for o, a in opts:

    if len(args) != 2:
        # should just be the one input file.
        print usage
        sys.exit(1)

    # retrieve commandline parms
    inName  = args[0]
    outName = args[1]

    # open and read XML Asymmetry file
    xmlFile = calCalibXML.calAsymCalibXML(inName)
    (xpos, asymData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # create output file
    rootFile = ROOT.TFile(outName, "RECREATE")

    # associate pos_diode, big_diode & val_or_sigma
    # to index into calAsymCalibXML array as spec'd as
    # follows (from calCalibXML.py)

    #     The next-to-last
    #     dimension contains the following data:
    #     0 = bigVals value
    #     1 = smallVals value
    #     2 = NsmallPbigVals value
    #     3 = PsmallNbigVals value
    #     4 = bigSigs value
    #     5 = smallSigs value
    #     6 = NsmallPbigSigs value
    #     7 = PsmallNbigSigs value

    # print header as comment

    # print out txt file.
    for twr in towers:
        for lyr in range(8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(12):
                for pdiode in range(2):
                    for ndiode in range(2):
                        # plot spline method
                        channel_str = "T%dL%dC%d_p%dn%d"%(twr,lyr,col,pdiode,ndiode)
                        spline = ROOT.TSpline3(channel_str,
                                               array.array('d',xpos),
                                               array.array('d',asymData[twr][row][col][cgc_util.asymIdx[(pdiode,ndiode,False)]]),
                                               len(xpos))

                        c = ROOT.TCanvas(channel_str, channel_str,-1)

                        spline.Draw()

                        g = ROOT.TGraphErrors(len(xpos), 
                                              array.array('d',xpos), 
                                              array.array('d',asymData[twr][row][col][cgc_util.asymIdx[(pdiode,ndiode,False)]]), 
                                              array.array('d',Numeric.zeros(len(xpos))), 
                                              array.array('d',asymData[twr][row][col][cgc_util.asymIdx[(pdiode,ndiode,True)]]))

                        g.Draw("*")
                                         
                                         
                                         
                        # plot actual points with 'error' bars
                        c.Write()

                        

    rootFile.Close()
