"""
Dump GLAST Cal offline tholdci calibration xml into column delmited text on stdout

output format is:
twr, lyr, col, face, lac, fle, fhe, uld0, uld1, uld2, uld3, ped0, ped1, ped2, ped3

python tholdciXML2TXT.py [-d delim] <input_xml_file>

where:
    <input_xml_file> = input tholdCI GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump tholdci file to .txt file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/02/21 23:37:19 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import Numeric

import calCalibXML
import calConstant


                  
if __name__ == '__main__':
    usage = "usage: python tholdciXML2TXT.py [-d delim] <input_xml_file>"

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

    # open and read XML Thold_CI file

    xmlFile = calCalibXML.calTholdCICalibXML(inName)
    (adcData, uldData, pedData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # Convert offline xtal face indexing to online xtal face indexing  (they're reversed, don't blame me :)
    offline_face_to_online = {0:1,1:0}

    # print out txt file.
    for twr in towers:
        for lyr in range(0,8):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(0,12):
                for face in range(0,2):
                    online_face = offline_face_to_online[face]
                    print delim.join([str(x) for x in twr, lyr, col, face,\
                                     adcData[twr][row][online_face][col][0],
                                     adcData[twr][row][online_face][col][1],\
                                     adcData[twr][row][online_face][col][2],\
                                     uldData[twr][row][online_face][col][0],\
                                     uldData[twr][row][online_face][col][1],\
                                     uldData[twr][row][online_face][col][2],\
                                     uldData[twr][row][online_face][col][3],\
                                     pedData[twr][row][online_face][col][0],\
                                     pedData[twr][row][online_face][col][1],\
                                     pedData[twr][row][online_face][col][2],\
                                     pedData[twr][row][online_face][col][3]])

 

 
 


 
