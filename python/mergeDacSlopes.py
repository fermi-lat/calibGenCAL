"""
Tool to produce CAL DAC XML calibration data file from TXT files for each discriminator type

mergeDacSlopes [-V] [-L <log_file>] [-doptionaldtd] <lac_slopes_txt> <fle_slopes_txt> <fhe_slopes_txt> <uld_slopes_txt> <output_xml>

where:
    -V               = verbose; turn on debug output
    -L <log_file>    = save console output to log text file
    -d               = specify path to optional dtd file
    <lac_slopes_txt> = path to input lac slopes file
    <fle_slopes_txt> = path to input fle slopes file
    <fhe_slopes_txt> = path to input fhe slopes file
    <uld_slopes_txt> = path to input uld slopes file
    <output_xml>     = output xml path

INPUTS:

	LAC, FLE, FHE:
        Slopes are expressed in MeV/DAC unit.
        TXT file columns are space delimited, ';' in 1st character on line indicates comment
        All Cal component indexing uses GLAST offline software conventions (POS_FACE = 0, layers numberd 0-7 along LAT z axis in direction away from tracker.)
        TXT format is as follows:
        "tower layer column face slope slope_error dac_range"

        where for dac_range, 0 = FINE and 1 = COARSE

        ULD:
        Slopes are expressed in MeV/DAC unit.
        TXT file columns are space delimited, ';' in 1st character on line indicates comment
        All Cal component indexing uses GLAST offline software conventions (POS_FACE = 0, layers numberd 0-7 along LAT z axis in direction away from tracker.)
        TXT format is as follows:
        "tower layer column face adc_range slope slope_error dac_range saturation_mev"

        where for dac_range, 0 = FINE and 1 = COARSE


OUTPUTS:
	one offline Cal dacSlopes XML file.

"""


__facility__  = "Offline"
__abstract__  = "Tool to produce CAL DAC XML calibration data file from TXT files for each discriminator type"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/04/21 14:36:57 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

def read_slopes_file(txt_path):
    """
    Input:
    Read in dac slopes file w/ following format:
    All Cal component indexing uses GLAST offline software conventions (POS_FACE = 0, layers numberd 0-7 along LAT z axis in direction away from tracker.)
    TXT format is as follows:
    "tower layer column face slope slope_error dac_range"
    
    where for dac_range, 0 = FINE and 1 = COARSE

    Output:
    tuple(slopes, offsets, dac_rng, twrSet)
    where:
      slopes, offsets, dac_rng are numarray arrays of shape (16,8,2,12)
      twrSet - set of towers included in data set
          
    """
    inFile = open(txt_path,'r')
    lines = inFile.readlines()
    
    import numarray
    slopeData  = numarray.zeros((16,8,2,12), numarray.Float32)
    offsetData = numarray.zeros((16,8,2,12), numarray.Float32)
    rngData    = numarray.zeros((16,8,2,12), numarray.Int8)
    twrSet = set()

    # loop over each line in code
    import calCalibXML
    import calConstant
    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue
    
        # read in values from current line
        [twr, offline_lyr, col, face, 
         slope, offset, dac_rng] = line.split()


        # convert array index values to integer.
        twr = int(twr)
        offline_lyr = int(offline_lyr)
        col = int(col)
        face = int(face)
        slope = float(slope)
        offset = float(offset)
        dac_rng = int(dac_rng)

        # make sure current tower is on list
        twrSet.add(twr)

        # get online row indexing (as opposed to offline lyr indexing)
        online_row = calCalibXML.layerToRow(offline_lyr)
        # convert offline face numbering to online face numbering
        online_face = calConstant.offline_face_to_online[face]

        slopeData[twr][online_row][online_face][col] = slope
        offsetData[twr][online_row][online_face][col] = offset
        rngData[twr][online_row][online_face][col] = dac_rng

    return (slopeData, offsetData, rngData, twrSet)

def read_uld_slopes_file(txt_path):
    """
    Input:
    Read in ULD dac slopes file w/ following format:
    All Cal component indexing uses GLAST offline software conventions (POS_FACE = 0, layers numberd 0-7 along LAT z axis in direction away from tracker.)
    TXT format is as follows:
    "tower layer column face adc_rng slope slope_error dac_range"
    
    where for dac_range, 0 = FINE and 1 = COARSE

    Output:
    tuple(slopes, offsets, dac_rng, saturation, twrSet)
    where:
      slopes, offsets, dac_rng, saturation are numarray arrays of shape (3,16,8,2,12)
      twrSet - set of towers included in data set
          
    """
    inFile = open(txt_path,'r')
    lines = inFile.readlines()
    
    import numarray
    slopeData  = numarray.zeros((3,16,8,2,12), numarray.Float32)
    offsetData = numarray.zeros((3,16,8,2,12), numarray.Float32)
    rngData    = numarray.zeros((3,16,8,2,12), numarray.Int8)
    satData    = numarray.zeros((3,16,8,2,12), numarray.Float32)
    twrSet = set()

    # loop over each line in code
    import calCalibXML
    import calConstant
    nLine = -1
    for line in lines:
        nLine+=1

        # skip comments
        if line[0] == ';':
            continue
    
        # read in values from current line
        [twr, offline_lyr, col, face, rng,
         slope, offset, dac_rng, saturation] = line.split()


        # convert array index values to integer.
        twr = int(twr)
        offline_lyr = int(offline_lyr)
        col = int(col)
        face = int(face)
        rng = int(rng)
        slope = float(slope)
        offset = float(offset)
        dac_rng = int(dac_rng)
        saturation = float(saturation)

        # make sure current tower is on list
        twrSet.add(twr)

        # get online row indexing (as opposed to offline lyr indexing)
        online_row = calCalibXML.layerToRow(offline_lyr)
        # convert offline face numbering to online face numbering
        online_face = calConstant.offline_face_to_online[face]

        slopeData[rng][twr][online_row][online_face][col]  = slope
        offsetData[rng][twr][online_row][online_face][col] = offset
        rngData[rng][twr][online_row][online_face][col]    = dac_rng
        satData[rng][twr][online_row][online_face][col]    = saturation

    return (slopeData, offsetData, rngData, satData, twrSet)

if __name__ == '__main__':
    # setup logger
    import logging

    logging.basicConfig()
    log = logging.getLogger('mergeDacSlopes')
    log.setLevel(logging.INFO) 
    
    # get environment settings
    import os
    try:
        calibUtilRoot = os.environ['CALIBGENCALROOT']
    except KeyError:
        log.error('CALIBGENCALROOT must be defined')
        sys.exit(1)

    # check command line
    import getopt
    import sys
    try:
        opts = getopt.getopt(sys.argv[1:], "-V-L:-d:")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    optList = opts[0]
    dtdPath = os.environ['CALIBGENCALROOT'] + '/xml/calCalib_v2r3.dtd'
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('Deleting old log file %s', o[1])
                os.remove(o[1])
            hdl = logging.FileHandler(o[1])
            fmt = logging.Formatter('%(levelname)s %(message)s')
            hdl.setFormatter(fmt)
            log.addHandler(hdl)
        elif o[0] == '-d':
            dtdPath = o[1]
        else:
            log.error("Invalid command line switch %s"%o)
            sys.exit(-1)
        
    args = opts[1]
    if len(args) != 5:
        log.error(__doc__)
        sys.exit(1)

    (lac_txt_path, fle_txt_path, fhe_txt_path, uld_txt_path, output_xml_path) = args

    (lac_slopes,  lac_offsets,  lac_rng, lacTwrSet) = read_slopes_file(lac_txt_path)
    (fle_slopes,  fle_offsets,  fle_rng, fleTwrSet) = read_slopes_file(fle_txt_path)
    (fhe_slopes,  fhe_offsets,  fhe_rng, fheTwrSet) = read_slopes_file(fhe_txt_path)
    (uld_slopes,  uld_offsets,  uld_rng, uld_saturation, uldTwrSet) = read_uld_slopes_file(uld_txt_path)

    # process only towers that have all data specified.
    twrSet = lacTwrSet & fleTwrSet & fheTwrSet & uldTwrSet
    log.info("Processing towers: %s"% twrSet)
 
    # create empty output data arrays
    import numarray
    import calConstant
    dacDataOut = numarray.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 12), numarray.Float32)
    uldDataOut = numarray.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 6), numarray.Float32)
    rngDataOut = numarray.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 6), numarray.Int8)         

    # insert data into output array
    import calCalibXML
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_SLOPE] = lac_slopes
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_LACDAC_OFFSET] = lac_offsets
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_SLOPE] = fle_slopes
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_FLEDAC_OFFSET] = fle_offsets
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_SLOPE] = fhe_slopes
    dacDataOut[..., calCalibXML.calDacSlopesCalibXML.DACDATA_FHEDAC_OFFSET] = fhe_offsets

    uldDataOut[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SLOPE] = uld_slopes
    uldDataOut[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_OFFSET] = uld_offsets
    uldDataOut[..., calCalibXML.calDacSlopesCalibXML.ULDDATA_SAT] = uld_saturation

    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_LACDAC] = lac_rng
    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FLEDAC] = fle_rng
    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_FHEDAC] = fhe_rng
    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX8] = uld_rng[0,...]
    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_LEX1] = uld_rng[1,...]
    rngDataOut[..., calCalibXML.calDacSlopesCalibXML.RNGDATA_ULDDAC_HEX8] = uld_rng[2,...]
    
    # create output file
    log.info("Creating file %s", output_xml_path)
    fio = calCalibXML.calDacSlopesCalibXML(output_xml_path, calCalibXML.MODE_CREATE)

    # put parameters into comments
    doc = fio.getDoc()
    c = doc.createComment("LAC slopes file: %s" % lac_txt_path)
    doc.appendChild(c)
    c = doc.createComment("FLE slopes file: %s" % fle_txt_path)
    doc.appendChild(c)
    c = doc.createComment("FHE slopes file: %s" % fhe_txt_path)
    doc.appendChild(c)
    c = doc.createComment("ULD slopes file: %s" % uld_txt_path)
    doc.appendChild(c)

    fio.write(dacDataOut, uldDataOut, rngDataOut, list(twrSet))
    fio.close()  
    
    # fixup calibration XML file - insert DTD info
    log.info("Inserting dtd file: %s"%dtdPath)
    calCalibXML.insertDTD(output_xml_path, dtdPath) 
                
    sys.exit(0)



