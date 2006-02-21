"""
Dump GLAST Cal offline tholdci calibration xml into column delmited text on stdout

output format is:
twr, lyr, col, face, lac, fle, fhe, uld0, uld1, uld2, uld3, ped0, ped1, ped2, ped3

tholdci2CSV <input_xml_file>
"""


__facility__  = "Offline"
__abstract__  = "Dump tholdci file to .csv file"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/01/30 21:57:20 $"
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

    usage = "usage: tholdci2CSV <input_xml_file>"

    # retrieve commandline parms
    inName  = sys.argv[1]

    # open and read XML Thold_CI file

    xmlFile = calCalibXML.calTholdCICalibXML(inName)
    (adcData, uldData, pedData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out csv file.
    for twr in towers:
        for lyr in range(0,8):
            for col in range(0,12):
                for face in range(0,2):
                    print twr, lyr, col, face,  ',',\
                    adcData[twr][lyr][face][col][0], ',',\
                    adcData[twr][lyr][face][col][1], ',',\
                    adcData[twr][lyr][face][col][2], ',',\
                    uldData[twr][lyr][face][col][0], ',',\
                    uldData[twr][lyr][face][col][1], ',',\
                    uldData[twr][lyr][face][col][2], ',',\
                    uldData[twr][lyr][face][col][3], ',',\
                    pedData[twr][lyr][face][col][0], ',',\
                    pedData[twr][lyr][face][col][1], ',',\
                    pedData[twr][lyr][face][col][2], ',',\
                    pedData[twr][lyr][face][col][3]

                        

                        
    


    
