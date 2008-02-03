"""
Convert GLAST Cal offline dacSlopes calibration xml into ROOT TTree format

python dacSlopesXML2TXT.py [-d delim] <input_xml_file> <output_root_file>

where:
    <input_xml_file> = input dacSlopes GLAST Cal offline calibration file
    <output_root_file> = will contain ROOT TTree with calibration data
"""


__facility__  = "Offline"
__abstract__  = "Convert dacSlopes XML file to .txt file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2008/02/02 23:50:49 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
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
    import logging
    # setup logger
    logging.basicConfig()
    log = logging.getLogger('dacSlopesXML2Ntuple')
    log.setLevel(logging.INFO)


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

    if len(args) != 2:
        # should just be the one input file.
        print "no input file specified: ", __doc__
        sys.exit(1)

    # retrieve commandline parms
    inPath  = args[0]
    outPath = args[1]

    # open and read XML dacSlopes file
    xmlFile = calCalibXML.calDacSlopesCalibXML(inPath)
    (dacData, uldData, rngData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # open ROOT output file, create ROOT output tree
    import ROOT
    log.info("Opening ROOT file: " + outPath)
    rootFile = ROOT.TFile(outPath,"RECREATE")
    t = ROOT.TTree("calDacSlopes", "calDacSlopes")
    
    # create ttree target values (arrays of length 1)
    import array
    _twr = array.array('B', [0])
    _lyr = array.array('B', [0])
    _col = array.array('B', [0])
    _face = array.array('B', [0])

    lac_slope = array.array('H', [0])
    lac_offest = array.array('H', [0])
    fle_slope = array.array('H', [0])
    fle_offset = array.array('H', [0])
    fhe_slope = array.array('H', [0])
    fhe_offset = array.array('H', [0])

    lac_slope_err = array.array('H', [0])
    lac_offset_err = array.array('H', [0])
    fle_slope_err = array.array('H', [0])
    fle_offset_err = array.array('H', [0])
    fhe_slope_err = array.array('H', [0])
    fhe_offset_err = array.array('H', [0])

    lex8_uld_slope = array.array('H', [0])
    lex8_uld_offset = array.array('H', [0])
    lex8_uld_saturation = array.array('H', [0])
    lex8_uld_slope_err = array.array('H', [0])
    lex8_uld_offset_err = array.array('H', [0])
    lex8_uld_saturation_err = array.array('H', [0])

    lex1_uld_slope = array.array('H', [0])
    lex1_uld_offset = array.array('H', [0])
    lex1_uld_saturation = array.array('H', [0])
    lex1_uld_slope_err = array.array('H', [0])
    lex1_uld_offset_err = array.array('H', [0])
    lex1_uld_saturation_err = array.array('H', [0])

    hex8_uld_slope = array.array('H', [0])
    hex8_uld_offset = array.array('H', [0])
    hex8_uld_saturation = array.array('H', [0])
    hex8_uld_slope_err = array.array('H', [0])
    hex8_uld_offset_err = array.array('H', [0])
    hex8_uld_saturation_err = array.array('H', [0])

    lac_dac_rng = array.array('B', [0])
    fle_dac_rng = array.array('B', [0])
    fhe_dac_rng = array.array('B', [0])
    lex8_uld_rng = array.array('B', [0])
    lex1_uld_rng = array.array('B', [0])
    hex8_uld_rng = array.array('B', [0])

    # create ttree branches
    t.Branch("twr",_twr,"twr/B")
    t.Branch("lyr",_lyr, 'lyr/B')
    t.Branch("col",_col, 'col/B')
    t.Branch("face",_face, 'face/B')

    t.Branch("lac_slope",lac_slope, 'lac_slope/H')
    t.Branch("lac_offest",lac_offest, 'lac_offest/H')
    t.Branch("fle_slope",fle_slope, 'fle_slope/H')
    t.Branch("fle_offset",fle_offset, 'fle_offset/H')
    t.Branch("fhe_slope",fhe_slope, 'fhe_slope/H')
    t.Branch("fhe_offset",fhe_offset, 'fhe_offset/H')

    t.Branch("lac_slope_err",lac_slope_err, 'lac_slope_err/H')
    t.Branch("lac_offset_err",lac_offset_err, 'lac_offset_err/H')
    t.Branch("fle_slope_err",fle_slope_err, 'fle_slope_err/H')
    t.Branch("fle_offset_err",fle_offset_err, 'fle_offset_err/H')
    t.Branch("fhe_slope_err",fhe_slope_err, 'fhe_slope_err/H')
    t.Branch("fhe_offset_err",fhe_offset_err, 'fhe_offset_err/H')

    t.Branch("lex8_uld_slope",lex8_uld_slope, 'lex8_uld_slope/H')
    t.Branch("lex8_uld_offset",lex8_uld_offset, 'lex8_uld_offset/H')
    t.Branch("lex8_uld_saturation",lex8_uld_saturation, 'lex8_uld_saturation/H')
    t.Branch("lex8_uld_slope_err",lex8_uld_slope_err, 'lex8_uld_slope_err/H')
    t.Branch("lex8_uld_offset_err",lex8_uld_offset_err, 'lex8_uld_offset_err/H')
    t.Branch("lex8_uld_saturation_err",lex8_uld_saturation_err, 'lex8_uld_saturation_err/H')

    t.Branch("lex1_uld_slope",lex1_uld_slope, 'lex1_uld_slope/H')
    t.Branch("lex1_uld_offset",lex1_uld_offset, 'lex1_uld_offset/H')
    t.Branch("lex1_uld_saturation",lex1_uld_saturation, 'lex1_uld_saturation/H')
    t.Branch("lex1_uld_slope_err",lex1_uld_slope_err, 'lex1_uld_slope_err/H')
    t.Branch("lex1_uld_offset_err",lex1_uld_offset_err, 'lex1_uld_offset_err/H')
    t.Branch("lex1_uld_saturation_err",lex1_uld_saturation_err, 'lex1_uld_saturation_err/H')

    t.Branch("hex8_uld_slope",hex8_uld_slope, 'hex8_uld_slope/H')
    t.Branch("hex8_uld_offset",hex8_uld_offset, 'hex8_uld_offset/H')
    t.Branch("hex8_uld_saturation",hex8_uld_saturation, 'hex8_uld_saturation/H')
    t.Branch("hex8_uld_slope_err",hex8_uld_slope_err, 'hex8_uld_slope_err/H')
    t.Branch("hex8_uld_offset_err",hex8_uld_offset_err, 'hex8_uld_offset_err/H')
    t.Branch("hex8_uld_saturation_err",hex8_uld_saturation_err, 'hex8_uld_saturation_err/H')

    t.Branch("lac_dac_rng",lac_dac_rng, 'lac_dac_rng/B')
    t.Branch("fle_dac_rng",fle_dac_rng, 'fle_dac_rng/B')
    t.Branch("fhe_dac_rng",fhe_dac_rng, 'fhe_dac_rng/B')
    t.Branch("lex8_uld_rng",lex8_uld_rng, 'lex8_uld_rng/B')
    t.Branch("lex1_uld_rng",lex1_uld_rng, 'lex1_uld_rng/B')
    t.Branch("hex8_uld_rng",hex8_uld_rng, 'hex8_uld_rng/B')

    # fill tree
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    
                    _twr[0] = twr
                    _lyr[0] = lyr
                    _col[0] = col
                    _face[0] = face

                    lac_slope[0] =                       dacData[twr][row][online_face][col][0]
                    lac_offest[0] =                      dacData[twr][row][online_face][col][1]
                    fle_slope[0] =                       dacData[twr][row][online_face][col][2]
                    fle_offset[0] =                      dacData[twr][row][online_face][col][3]
                    fhe_slope[0] =                       dacData[twr][row][online_face][col][4]
                    fhe_offset[0] =                      dacData[twr][row][online_face][col][5]
                    lac_slope_err[0] =                   dacData[twr][row][online_face][col][6]
                    lac_offset_err[0] =                  dacData[twr][row][online_face][col][7]
                    fle_slope_err[0] =                   dacData[twr][row][online_face][col][8]
                    fle_offset_err[0] =                  dacData[twr][row][online_face][col][9]
                    fhe_slope_err[0] =                   dacData[twr][row][online_face][col][10]
                    fhe_offset_err[0] =                  dacData[twr][row][online_face][col][11]
                                                         
                    lex8_uld_slope[0] =                  uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][0]
                    lex8_uld_offset[0] =                 uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][1]
                    lex8_uld_saturation[0] =             uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][2]
                    lex8_uld_slope_err[0] =              uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][3]
                    lex8_uld_offset_err[0] =             uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][4]
                    lex8_uld_saturation_err[0] =         uldData[calConstant.CRNG_LEX8][twr][row][online_face][col][5]
                                                         
                    lex1_uld_slope[0] =                  uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][0]
                    lex1_uld_offset[0] =                 uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][1]
                    lex1_uld_saturation[0] =             uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][2]
                    lex1_uld_slope_err[0] =              uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][3]
                    lex1_uld_offset_err[0] =             uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][4]
                    lex1_uld_saturation_err[0] =         uldData[calConstant.CRNG_LEX1][twr][row][online_face][col][5]
                                                         
                    hex8_uld_slope[0] =                  uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][0]
                    hex8_uld_offset[0] =                 uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][1]
                    hex8_uld_saturation[0] =             uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][2]
                    hex8_uld_slope_err[0] =              uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][3]
                    hex8_uld_offset_err[0] =             uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][4]
                    hex8_uld_saturation_err[0] =         uldData[calConstant.CRNG_HEX8][twr][row][online_face][col][5]
                                                         
                    lac_dac_rng[0] =                     rngData[twr][row][online_face][col][0]
                    fle_dac_rng[0] =                     rngData[twr][row][online_face][col][1]
                    fhe_dac_rng[0] =                     rngData[twr][row][online_face][col][2]
                    lex8_uld_rng[0] =                    rngData[twr][row][online_face][col][3]
                    lex1_uld_rng[0] =                    rngData[twr][row][online_face][col][4]
                    hex8_uld_rng[0] =                    rngData[twr][row][online_face][col][5]

                    T.Fill()


    t.Write()
