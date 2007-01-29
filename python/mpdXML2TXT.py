"""
Dump GLAST Cal offline mevPerDAC calibration xml into column delmited text on stdout

output format is:
twr, lyr, col, diode, mevPerDAC, sigma

mpdXML2TXT [-d delim] <input_xml_file>

where:
    <input_xml_file> = input mevPerDAC  GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump offline mevPerDAC xml file to .txt file"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/08/03 13:11:03 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import Numeric

import calCalibXML
import calConstant


                  
if __name__ == '__main__':
    usage = "usage: python mpdXML2TXT.py [-d delim] <input_xml_file>"

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

    if len(args) != 1:
        # should just be the one input file.
        print "no input file specified: ", usage
        sys.exit(1)

    # retrieve commandline parms
    inName  = args[0]

    # open and read XML MevPerDAC file
    xmlFile = calCalibXML.calMevPerDacCalibXML(inName)
    mpdData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_ROW):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for diode in range(calConstant.NUM_DIODE):
                    # from calCalibXML doc, array layout is as follows
                    #Returns: A Numeric array containing the energy conversion data
                    #         of shape (16, 8, 12, 8) The last dimension contains
                    #         the following data for each crystal:
                    #             0 = bigVal value
                    #             1 = smallVal value
                    #             2 = bigSig value
                    #             3 = smallSig value2 = bigSig value
                    print delim.join([str(x) for x in twr, lyr, col, diode,\
                                      mpdData[twr][row][col][diode],\
                                      mpdData[twr][row][col][2+diode]])
