"""
Dump GLAST Cal offline dacSlopes calibration xml into column delmited text on stdout

output format is one line per crystal face:
twr lyr col face lac_slope lac_offest fle_slope fle_offset fhe_slope fhe_offset lac_slope_err lac_offset_err fle_slope_err fle_offset_err fhe_slope_err fhe_offset_err lex8_uld_slope lex8_uld_offset lex8_uld_saturation lex8_uld_slope_err lex8_uld_offset_err lex8_uld_saturation_err lex1_uld_slope lex1_uld_offset lex1_uld_saturation lex1_uld_slope_err lex1_uld_offset_err lex1_uld_saturation_err hex8_uld_slope hex8_uld_offset hex8_uld_saturation hex8_uld_slope_err hex8_uld_offset_err hex8_uld_saturation_err lac_dac_rng fle_dac_rng fhe_dac_rng lex8_uld_rng lex1_uld_rng hex8_uld_rng

python dacSlopesXML2TXT.py [-d delim] <input_xml_file>

where:
    <input_xml_file> = input dacSlopes GLAST Cal offline calibration file
        -d delim         = optional field delimeter override (default = ' ')
"""


__facility__  = "Offline"
__abstract__  = "Dump dacSlopes XML file to .txt file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2007/09/13 18:31:45 $"
__version__   = "$Revision: 1.9 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, math
import getopt
import array

import Numeric

import calCalibXML
import calConstant
import cgc_util

                  
if __name__ == '__main__':
    usage = "usage: python dacSlopesXML2TXT.py [-d delim] <input_xml_file>"

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

    # open and read XML dacSlopes file

    xmlFile = calCalibXML.calDacSlopesCalibXML(inName)
    (dacData, uldData, rngData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # print out header as comment
    print ";twr lyr col face lac_slope lac_offest fle_slope fle_offset fhe_slope fhe_offset lac_slope_err lac_offset_err fle_slope_err fle_offset_err fhe_slope_err fhe_offset_err lex8_uld_slope lex8_uld_offset lex8_uld_saturation lex8_uld_slope_err lex8_uld_offset_err lex8_uld_saturation_err lex1_uld_slope lex1_uld_offset lex1_uld_saturation lex1_uld_slope_err lex1_uld_offset_err lex1_uld_saturation_err hex8_uld_slope hex8_uld_offset hex8_uld_saturation hex8_uld_slope_err hex8_uld_offset_err hex8_uld_saturation_err lac_dac_rng fle_dac_rng fhe_dac_rng lex8_uld_rng lex1_uld_rng hex8_uld_rng"

    # print out txt file.
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_ROW):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    print delim.join([
                            str(x) for x in twr, lyr, col, face,\
                                dacData[twr][row][online_face][col][0],\
                                dacData[twr][row][online_face][col][1],\
                                dacData[twr][row][online_face][col][2],\
                                dacData[twr][row][online_face][col][3],\
                                dacData[twr][row][online_face][col][4],\
                                dacData[twr][row][online_face][col][5],\
                                dacData[twr][row][online_face][col][6],\
                                dacData[twr][row][online_face][col][7],\
                                dacData[twr][row][online_face][col][8],\
                                dacData[twr][row][online_face][col][9],\
                                dacData[twr][row][online_face][col][10],\
                                dacData[twr][row][online_face][col][11],\
                                
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][0],\
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][1],\
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][2],\
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][3],\
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][4],\
                                uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][5],\

                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][0],\
                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][1],\
                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][2],\
                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][3],\
                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][4],\
                                uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][5],\

                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][0],\
                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][1],\
                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][2],\
                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][3],\
                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][4],\
                                uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][5],\

                                rngData[twr][row][online_face][col][0],\
                                rngData[twr][row][online_face][col][1],\
                                rngData[twr][row][online_face][col][2],\
                                rngData[twr][row][online_face][col][3],\
                                rngData[twr][row][online_face][col][4],\
                                rngData[twr][row][online_face][col][5]
                            ])
