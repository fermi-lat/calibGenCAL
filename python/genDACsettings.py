"""
Generate DAC (FHE, FLE, ULD, or LAC) settings XML files using a CAL DacSlopes file as a calibration
reference.  The command line is:

genDACsettings [-V] [-G <gain>] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_xml_file> <dac_xml_file>

where:
    -V              = verbose; turn on debug output
    -G <gain>       = gain setting (default is 5 for FLE and LAC, 15 for FHE)      
    FLE|FHE|LAC|ULD = DAC type to validate
    <MeV | margin>  = For FLE, FHE, LAC: The threshold energy in MeV units.
                      For ULD: The margin as a percentage of the saturation value.
    <dac_slopes_xml_file> = The CAL_DacSlopes calibration file.                     
    <dac_xml_file>  = The DAC settings XML file output name.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2007/08/30 21:15:56 $"
__version__     = "$Revision: 1.8 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



import sys
import logging
import getopt

import Numeric

import calCalibXML
import calDacXML
import calConstant
import calDacSettings


DAC_MAP = {'FLE' : 'fle_dac', 'FHE' : 'fhe_dac', 'LAC' : 'log_acpt', 'ULD' : 'rng_uld_dac'} 

    

if __name__ == '__main__':
    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genDACsettings')
    log.setLevel(logging.INFO)

    timetag = None

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V-t:-G:")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    gain = None

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-t':
            timetag = o[1]  
        elif o[0] == '-G':
            gain = int(o[1])  
                
    args = opts[1]
    if len(args) != 4:
        log.error(__doc__)
        sys.exit(1)

    dacType = args[0]
    MeV = float(args[1])
    dacSlopesName = args[2]
    outName = args[3]
    
    if dacType not in DAC_MAP.keys():
        log.error("DAC type %s not supported", dacType)
        sys.exit(1)

    log.debug('Using DAC type %s', dacType)
    if dacType != 'ULD':   
        log.debug('Using threshold %f MeV', MeV)
    else:
        log.debug('Using margin %f %%', MeV) 
           

    # read DAC to energy conversion file
    
    log.info("Reading file %s", dacSlopesName)
    fio = calCalibXML.calDacSlopesCalibXML(dacSlopesName)
    towers = fio.getTowers()
    (dacData, uldData, rangeData) = fio.read()
    fio.close()
    
    # generate settings bases on DAC linear fit data
    
    if dacType == 'LAC':
        settings = calDacSettings.dacSettings(dacData[...,0], dacData[...,1], MeV)
        settings = calDacSettings.setRange(settings, rangeData[...,0])
    elif dacType == 'FLE':
        settings = calDacSettings.dacSettings(dacData[...,2], dacData[...,3], MeV)
        settings = calDacSettings.setRange(settings, rangeData[...,1])
    elif dacType == 'FHE':
        settings = calDacSettings.dacSettings(dacData[...,4], dacData[...,5], MeV)
        settings = calDacSettings.setRange(settings, rangeData[...,2]) 
    else:
        settings = calDacSettings.uldSettings(uldData[...,0], uldData[...,1], uldData[...,2], (MeV / 100)) 
        settings = calDacSettings.setRange(settings, Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW,
            calConstant.NUM_END, calConstant.NUM_FE), Numeric.Int8))       
    
    # create output DAC settings XML file
    
    if dacType == 'ULD':
        adcmargin = MeV
        energy = None
    else:
        adcmargin = None
        energy = MeV
        
    if dacType == 'FHE':
        legain = None
        if gain is None:
            hegain = 15
        else:
            hegain = gain
    elif dacType == 'ULD':
        legain = None
        hegain = None
    else:
        if gain is None:
            legain = 5
        else:
            legain = gain
        hegain = None
        
    log.info("Creating file %s", outName)
    fio = calDacXML.calSettingsXML(outName, DAC_MAP[dacType], calDacXML.MODE_CREATE)
    fio.write(settings, tems = towers)
    fio.close() 
  
    log.debug("%s DAC\n%s", dacType, settings[0,...])  
    log.debug("%s DAC RANGE\n%s", dacType, (settings[0,...] >= 64))
 
    sys.exit(0)
     

   
