"""
Dump GLAST Cal offline pedestal calibration xml into column delmited text on stdout

output format is:
twr, lyr, col, face, range, pedestal, sigma

pedXML2TXT [-d delim] <input_xml_file>

where:
    <input_xml_file> = input pedestal  GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump offline pedestal xml file to .txt file"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/02/21 14:49:21 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import Numeric

import calCalibXML
import calConstant


                  
if __name__ == '__main__':
    usage = "usage: python pedXML2TXT.py [-d delim] <input_xml_file>"

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

    # open and read XML Pedestal file

    xmlFile = calCalibXML.calPedCalibXML(inName)
    pedData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out txt file.
    for twr in towers:
        for lyr in range(8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(12):
                for face in range(2):
                    for rng in range(4):
                        print delim.join([str(x) for x in twr, lyr, col, face, rng,
                                          pedData[twr][row][face][col][rng][0],
                                          pedData[twr][row][face][col][rng][1]])
                                          

 

 
 


 
