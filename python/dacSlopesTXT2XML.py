"""
Tool to generate CAL offline dacSlopes calibration XML file from space delimited TXT file.
Note: dacSlopesTXT2XML attempts to autodetect if input data is for partial LAT only (<16 towers).
Note: dacSlopesTXT2XML assumes 10 data points from xtal, one crystal width apart.

dacSlopesTXT2XML [-doptional.dtd] input.txt output.xml

where:
    -d           = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')

    input.txt    = dacSlopes input txt file, space delimited in following format: (one line per xtal face)
                   "twr lyr col face lac_slope lac_offest fle_slope fle_offset fhe_slope fhe_offset lac_slope_err lac_offset_err fle_slope_err fle_offset_err fhe_slope_err fhe_offset_err lex8_uld_slope lex8_uld_offset lex8_uld_saturation lex8_uld_slope_err lex8_uld_offset_err lex8_uld_saturation_err lex1_uld_slope lex1_uld_offset lex1_uld_saturation lex1_uld_slope_err lex1_uld_offset_err lex1_uld_saturation_err hex8_uld_slope hex8_uld_offset hex8_uld_saturation hex8_uld_slope_err hex8_uld_offset_err hex8_uld_saturation_err lac_dac_rng fle_dac_rng fhe_dac_rng lex8_uld_rng lex1_uld_rng hex8_uld_rng"
        
    output.xml = properly formatted offline CAL dacSlopes XML file
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate CAL DacSlopes calibration XML files from TXT."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:49 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import numarray
import sets
import array

import calCalibXML
import calConstant
import cgc_util

#######################################################################################3

if __name__ == '__main__':

    #constants
    dtdName    = "calCalib_v2r3.dtd" #default value
 
    # setup logger
    logging.basicConfig()
    log = logging.getLogger('dacSlopesTXT2XML')
    log.setLevel(logging.INFO)


    # get environment settings
    try:
        calibGenCALRoot = os.environ['CALIBGENCALROOT']
    except:
        log.error('CALIBGENCALROOT must be defined')
        sys.exit(1)    

    # parse commandline
    #  - code stolen from: http://python.active-venture.com/lib/module-getopt.html
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:")
    except getopt.GetoptError:
        log.exception("getopt exception: " + __doc__)
        sys.exit(-1)

    for o, a in opts:
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibGenCALRoot, 'xml', dtdName)

    if (len(args) != 2):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)


    inPath = args[0]
    outPath = args[1]


    # create empty data arrays
        
    dacData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW,
                             calConstant.NUM_END, calConstant.NUM_FE, 12), numarray.Float32)
    uldData = numarray.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW,
                             calConstant.NUM_END, calConstant.NUM_FE, 6), numarray.Float32)
    rngData = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW,
                             calConstant.NUM_END, calConstant.NUM_FE, 6), numarray.Int16)

    # keep track of active towers
    twrSet = set()

    # read input files
    inFile = open(inPath, 'r')
    lines = inFile.readlines()

    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue

        # read in values from current line
        [twr, lyr, col, face, 

         lac_slope, lac_offest, 
         fle_slope, fle_offset, 
         fhe_slope, fhe_offset, 
         lac_slope_err, lac_offset_err, 
         fle_slope_err, fle_offset_err, 
         fhe_slope_err, fhe_offset_err, 
         
         lex8_uld_slope, lex8_uld_offset, lex8_uld_saturation, lex8_uld_slope_err, lex8_uld_offset_err, lex8_uld_saturation_err, 
         lex1_uld_slope, lex1_uld_offset, lex1_uld_saturation, lex1_uld_slope_err, lex1_uld_offset_err, lex1_uld_saturation_err, 
         hex8_uld_slope, hex8_uld_offset, hex8_uld_saturation, hex8_uld_slope_err, hex8_uld_offset_err, hex8_uld_saturation_err, 

         lac_dac_rng, fle_dac_rng, fhe_dac_rng, lex8_uld_rng, lex1_uld_rng, hex8_uld_rng] = line.split()

        # convert array index values to integer.
        twr = int(twr)
        lyr = int(lyr)
        col = int(col)
        face = int(face)

        # make sure current tower is on list
        twrSet.add(twr)

        # get online row indexing (as opposed to offline lyr indexing)
        row = calCalibXML.layerToRow(int(lyr))
        # convert offline face numbering to online face numbering
        online_face = calConstant.offline_face_to_online[face]


        dacData[twr][row][online_face][col][0] = float(  lac_slope     )
        dacData[twr][row][online_face][col][1] = float(  lac_offest    )
        dacData[twr][row][online_face][col][2] = float(  fle_slope     )
        dacData[twr][row][online_face][col][3] = float(  fle_offset    )
        dacData[twr][row][online_face][col][4] = float(  fhe_slope     )
        dacData[twr][row][online_face][col][5] = float(  fhe_offset    )
        dacData[twr][row][online_face][col][6] = float(  lac_slope_err )
        dacData[twr][row][online_face][col][7] = float(  lac_offset_err)
        dacData[twr][row][online_face][col][8] = float(  fle_slope_err )
        dacData[twr][row][online_face][col][9] = float(  fle_offset_err)
        dacData[twr][row][online_face][col][10] = float( fhe_slope_err )
        dacData[twr][row][online_face][col][11] = float( fhe_offset_err)
                                
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][0] = float( lex8_uld_slope         )
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][1] = float( lex8_uld_offset        )
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][2] = float( lex8_uld_saturation    )
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][3] = float( lex8_uld_slope_err     )
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][4] = float( lex8_uld_offset_err    )
        uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][5] = float( lex8_uld_saturation_err)

        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][0] = float( lex1_uld_slope         )
        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][1] = float( lex1_uld_offset        )
        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][2] = float( lex1_uld_saturation    )
        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][3] = float( lex1_uld_slope_err     )
        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][4] = float( lex1_uld_offset_err    )
        uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][5] = float( lex1_uld_saturation_err)

        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][0] = float( hex8_uld_slope         )
        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][1] = float( hex8_uld_offset        )
        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][2] = float( hex8_uld_saturation    )
        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][3] = float( hex8_uld_slope_err     )
        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][4] = float( hex8_uld_offset_err    )
        uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][5] = float( hex8_uld_saturation_err)

        rngData[twr][row][online_face][col][0] = int( lac_dac_rng )
        rngData[twr][row][online_face][col][1] = int( fle_dac_rng )
        rngData[twr][row][online_face][col][2] = int( fhe_dac_rng )
        rngData[twr][row][online_face][col][3] = int( lex8_uld_rng)
        rngData[twr][row][online_face][col][4] = int( lex1_uld_rng)
        rngData[twr][row][online_face][col][5] = int( hex8_uld_rng)

    log.info('Writing output file %s', outPath)

    outFile = calCalibXML.calDacSlopesCalibXML(outPath, calCalibXML.MODE_CREATE)
    outFile.write(dacData, uldData, rngData, tems = twrSet)
    outFile.close()

    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(outPath, dtdPath)
    sys.exit(0)                            
