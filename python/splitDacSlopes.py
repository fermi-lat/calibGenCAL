"""
Split dacSlopes XML file into individual text files with slope/offset models for each threhsold

Usage:
python dacSlopesXML2TXT.py [-d delim] <input_xml_file> <output_basename>

where:
    <input_xml_file>  = input dacSlopes GLAST Cal offline calibration file
    <output_basename> = each output file will be based on this name with additional extension.
    -d delim          = optional field delimeter override (default = ' ')


Outputs:
For LAC,FLE,FHE:
output format is one line per crystal face:
twr lyr col face dac_slope(mev/adc) dac_offest(mev) dac_range(0,1)

For ULD:
output format is one line per adc channel:
twr lyr col face range uld_slope(mev/adc) uld_offset(mev) uld_dac_range(0,1)

"""


__facility__  = "Offline"
__abstract__  = "Split dacSlopes XML file into individual TXT files"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2008/04/21 14:36:57 $"
__version__   = "$Revision: 1.5 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

                  
if __name__ == '__main__':
    # check commandline
    delim = ' '
    import getopt
    import sys
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
        print "no input file specified: ", __doc__
        sys.exit(1)

    # retrieve commandline parms
    (inName, outputBasename) = args

    # open and read XML dacSlopes file
    import calCalibXML
    xmlFile = calCalibXML.calDacSlopesCalibXML(inName)
    (dacData, uldData, rngData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # open output files
    lacFilename = outputBasename + '.lac_slope.txt'
    fleFilename = outputBasename + '.fle_slope.txt'
    fheFilename = outputBasename + '.fhe_slope.txt'
    uldFilename = outputBasename + '.uld_slope.txt'

    lacFile = open(lacFilename, 'w')
    fleFile = open(fleFilename, 'w')
    fheFile = open(fheFilename, 'w')
    uldFile = open(uldFilename, 'w')

    # print out headers as comment
    lacFile.write(";twr lyr col face lac_slope lac_offest lac_rng\n")
    fleFile.write(";twr lyr col face fle_slope fle_offest fle_rng\n")
    fheFile.write(";twr lyr col face fhe_slope fhe_offest fhe_rng\n")
    uldFile.write(";twr lyr col face uld_slope uld_offest uld_rng\n")

    # print out txt file.
    import calConstant
    from calCalibXML import *
    for twr in towers:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    lacFile.write(delim.join([
                        str(x) for x in twr, lyr, col, face,\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_LACDAC_SLOPE],\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_LACDAC_OFFSET],\
                        rngData[twr][row][online_face][col][calDacSlopesCalibXML.RNGDATA_LACDAC]]))
                    lacFile.write("\n")

                    fleFile.write(delim.join([
                        str(x) for x in twr, lyr, col, face,\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_FLEDAC_SLOPE],\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_FLEDAC_OFFSET],\
                        rngData[twr][row][online_face][col][calDacSlopesCalibXML.RNGDATA_FLEDAC]]))
                    fleFile.write("\n")

                    fheFile.write(delim.join([
                        str(x) for x in twr, lyr, col, face,\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_FHEDAC_SLOPE],\
                        dacData[twr][row][online_face][col][calDacSlopesCalibXML.DACDATA_FHEDAC_OFFSET],\
                        rngData[twr][row][online_face][col][calDacSlopesCalibXML.RNGDATA_FHEDAC]]))
                    fheFile.write("\n")

                    for rng in range(calConstant.NUM_RNG-1): # only process first 3 ranges
                        uldFile.write(delim.join([
                            str(x) for x in twr, lyr, col, face, rng,
                            uldData[rng][twr][row][online_face][col][calDacSlopesCalibXML.ULDDATA_SLOPE],\
                            uldData[rng][twr][row][online_face][col][calDacSlopesCalibXML.ULDDATA_OFFSET],\
                            uldData[rng][twr][row][online_face][col][calDacSlopesCalibXML.ULDDATA_SAT],\
                            rngData[twr][row][online_face][col][calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX8+rng]]))
                        uldFile.write("\n")
