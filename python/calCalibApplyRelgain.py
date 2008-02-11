"""
Tool to apply gain corrections from relgain.xml files to calCalib calibration files.

currently supported calCalib types are:
muSlope

The commandline is:
calCalibApplyRelgain [-V] [-doptional.dtd]  <relgain.xml> <le_gain_in> <le_gain_out> <he_gain_in> <he_gain_out> <input_calCalib.xml> <output_calCalib.xml>

where:
    -d         = specify which dtd file to load from calibUtil/xml folder (default = 'calCalib_v2r3.dtd')
    -V                   = verbose; turn on debug output
    <le_gain_in>         = le gain correpsonding to input xml file (0-8)
    <le_gain_out>        = output le gain (0-8)
    <he_gain_in>         = he gain corresponding to input xml file (0-8)
    <he_gain_out>        = output he gain (0-8)
    <relgain.xml>        = online gain correction file
    <input_calCalib.xml>  = offline calCalib calibration file (input)
    <output_calCalib.xml> = offline calCalib calibration file (output)
"""

__facility__  = "Offline"
__abstract__  = "Tool to convert calCalib xml file from one gain setting to another"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:49 $"
__version__   = "$Revision: 1.4 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import ConfigParser
import getopt
import numarray
import calFitsXML
import calCalibXML
from calConstant import *

if __name__ == '__main__':
    dtdName    = "calCalib_v2r3.dtd" #default value

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('calCalibApplyRelgain')
    log.setLevel(logging.INFO)

    # get environment settings
    try:
        calibUtilRoot = os.environ['CALIBUTILROOT']
    except:
        log.error('CALIBUTILROOT must be defined')
        sys.exit(1)    

    # check command line
    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    optList = opts[0]
    for o,a in optList:
        if o == '-V':
            log.setLevel(logging.DEBUG)
        if o == "-d":
            dtdName = a

    dtdPath = os.path.join(calibUtilRoot, 'xml', dtdName)

    args = opts[1]
    if len(args) != 7:
        log.error(__doc__)
        sys.exit(1)

    (relgain_filename,
     le_gain_in,
     le_gain_out,
     he_gain_in,
     he_gain_out,
     input_calCalib_filename,
     output_calCalib_filename) = args

    # convert gains to int
    le_gain_in  = int(le_gain_in)
    le_gain_out = int(le_gain_out)
    he_gain_in  = int(he_gain_in)
    he_gain_out = int(he_gain_out)

    # adjust he_gain to proper index
    he_gain_in_req = he_gain_in
    he_gain_out_req = he_gain_out

    # move muon-gain to highest index slot
    if he_gain_in == 0: he_gain_in = 16
    if he_gain_out == 0: he_gain_out = 16
    
    he_gain_in -= 8
    he_gain_out -= 8

    # open input files
    log.info("Opening %s for read."%relgain_filename)
    relgain_file = calFitsXML.calFitsXML(fileName = relgain_filename)
    relgain  = relgain_file.read()

    log.info("Opening %s for read."%input_calCalib_filename)
    input_slope_file = calCalibXML.calMuSlopeCalibXML(fileName = input_calCalib_filename)
    input_slope = input_slope_file.read()
    twrs = input_slope_file.getTowers()

    # apply corrections
    log.info("Applying LE relgain correction: %d to %d"%(le_gain_in, le_gain_out))
    for rng in (CRNG_LEX8, CRNG_LEX1):
        # relgain is basically measuring the inverse of adc2nrg for each
        # gain setting.  that is how i chose my '*' & '/' operations -zach
        input_slope[...,rng,:] *= relgain[le_gain_in,rng,...,numarray.NewAxis]
        input_slope[...,rng,:] /= relgain[le_gain_out,rng,...,numarray.NewAxis]


    log.info("Applying HE relgain correction: %d to %d (idx %d to %d)"% \
             (he_gain_in_req,he_gain_out_req,he_gain_in, he_gain_out))
    for rng in (CRNG_HEX8, CRNG_HEX1):
        # relgain is basically measuring the inverse of adc2nrg for each
        # gain setting.  that is how i chose my '*' & '/' operations -zach
        input_slope[...,rng,:] *= relgain[he_gain_in,rng,...,numarray.NewAxis]
        input_slope[...,rng,:] /= relgain[he_gain_out,rng,...,numarray.NewAxis]

    #save output file
    log.info("Opening %s for write."%output_calCalib_filename)
    output_slope_file = calCalibXML.calMuSlopeCalibXML(fileName = output_calCalib_filename,
                                                       mode=calCalibXML.MODE_CREATE
                                                       )
    output_slope_file.write(input_slope, tems=twrs)
    output_slope_file.close()
    # fixup calibration XML file - insert DTD info
    calCalibXML.insertDTD(output_calCalib_filename, dtdPath)




