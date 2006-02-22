"""
Dump GLAST Cal offline asymmetry calibration xml into column delmited text on stdout

output format is:
twr, lyr, col, face, pos_face_diode, neg_face_diode, xpos, asym, sig

asymXML2TXT [-d delim] <input_xml_file>

where:
    <input_xml_file> = input asymmetry GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump offline asymmetry xml file to .txt file"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2006/02/21 22:41:14 $"
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
    usage = "usage: python asymXML2TXT.py [-d delim] <input_xml_file>"

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

    xmlFile = calCalibXML.calAsymCalibXML(inName)
    (xpos, asymData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # indeces for outData
    valIdx = {'LL':0, 'SS':1, 'LS':2, 'SL':3}
    sigIdx = {'LL':4, 'SS':5, 'LS':6, 'SL':7}

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

    # tuple idx keys are (pos_diode, neg_diode, sigma)
    asymIdx = {
        (0,0,False):0,\
        (1,1,False):1,\
        (0,1,False):2,\
        (1,0,False):3,\
        (0,0,True):4,\
        (1,1,True):5,\
        (0,1,True):6,\
        (1,0,True):7}

    
    #print asymData
    #sys.exit()
    # print out txt file.
    for twr in towers:
        for lyr in range(8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(12):
                for pdiode in range(2):
                    for ndiode in range(2):
                        for pt in range (0,len(xpos)):
                            print delim.join([str(x) for x in twr, lyr, col,
                                              pdiode, ndiode, xpos[pt],
                                              asymData[twr][row][col][asymIdx[(pdiode,ndiode,False)]][pt], #asym val
                                              asymData[twr][row][col][asymIdx[(pdiode,ndiode,True)]][pt]])   #sigma
                                              



 

 
 


 
