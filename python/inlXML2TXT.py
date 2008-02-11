"""
Dump GLAST Cal offline Integral Nonlinearity (ADC<->CIDAC) calibration xml into column delmited text on stdout

output format is:
twr lyr col face range cidac adc

inlXML2TXT [-d delim] <input_xml_file>

where:
    <input_xml_file> = input intNonlin GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump offline intNonlin xml file to .txt file"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:49 $"
__version__   = "$Revision: 1.11 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import numarray

import calCalibXML
import calConstant
import cgc_util


                  
if __name__ == '__main__':

    # check commandline
    delim = ' '
    try:
        (opts,args) = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)
    
    # opts has 2 parts, options (-abc ...) & remaining default params
    for o, a in opts:
        if o == '-d':
            delim = a

    if len(args) != 1:
        # should just be the one input file.
        print "no input file specified: ", __doc__
        sys.exit(1)

    # retrieve commandline parms
    inName  = args[0]

    # open and read XML intNonlin file

    xmlFile = calCalibXML.calIntNonlinCalibXML(inName)
    (lenData, dacData, adcData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out header as comment
    print ";twr lyr col face range cidac adc"

    # print out txt file.
    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for rng in range(4):
                        for pt in range(lenData[rng][twr][row][online_face][col]):
                            print delim.join([str(x) for x in twr, lyr, col, face, rng,
                                              dacData[rng][twr][row][online_face][col][pt],
                                              adcData[rng][twr][row][online_face][col][pt]])
                                          

 

 
 


 
