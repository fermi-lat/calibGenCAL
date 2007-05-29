"""
Generate DAC (FHE, FLE, ULD, or LAC) settings XML files using a CAL DacSlopes file as a calibration
reference.  The command line is:

genDACsettings [-V] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_xml_file> <dac_xml_file> [<gain>]

where:
    -V              = verbose; turn on debug output
    FLE|FHE|LAC|ULD = DAC type to validate
    <MeV | margin>  = For FLE, FHE, LAC: The threshold energy in MeV units.
                      For ULD: The margin as a percentage of the saturation value.
    <dac_slopes_xml_file> = The CAL_DacSlopes calibration file.                     
    <dac_xml_file>  = The DAC settings XML file output name.
    <gain>          = CAL gain setting DAC's are valid for (needed for FLE, FHE, and LAC only). 
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/09/28 21:26:48 $"
__version__     = "$Revision: 1.5 $, $Author: dwood $"
__release__     = "$Name: v4r4 $"
__credits__     = "NRL code 7650"



import sys
import logging
import getopt

import Numeric

import calCalibXML
import calDacXML
import calConstant



DAC_MAP = {'FLE' : 'fle_dac', 'FHE' : 'fhe_dac', 'LAC' : 'log_acpt', 'ULD' : 'rng_uld_dac'} 




def dacSettings(slope, offset, energy):

     d = (energy - offset) / slope
     d = Numeric.clip(d, 0, 63)
     d = Numeric.around(d) 
     return d.astype(Numeric.Int8)
     
     
     
     
def uldSettings(slopes, offsets, sats, margin):

    sats = sats - (sats * margin)
    d = (sats - offsets) / slopes
    d = Numeric.clip(d, 0, 63)
    d = Numeric.around(d)
    lex8 = d[calConstant.CRNG_LEX8, ...]
    lex1 = d[calConstant.CRNG_LEX1, ...]  
    hex8 = d[calConstant.CRNG_HEX8, ...]
    s = Numeric.where((lex8 < lex1), lex8, lex1)
    s = Numeric.where((s < hex8), s, hex8)
    return s.astype(Numeric.Int8) 
              
     
     
     
def setRange(dacs, rangeData):

    return Numeric.where(rangeData, (dacs + 64), dacs)
          
  

    

if __name__ == '__main__':
    
    usage = \
        "genDACsettings [-V] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_xml_name> <out_xml_name> [<gain>]"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genDACsettings')
    log.setLevel(logging.INFO)

    timetag = None

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V-t:")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-t':
            timetag = o[1]    
                
    args = opts[1]
    if len(args) < 4:
        log.error(usage)
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
        if len(args) != 5:
            log.error(usage)
            sys.exit(1)
        gain = int(args[4])    
        log.debug('Using threshold %f MeV', MeV)
        log.debug('Using gain %d', gain)
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
        settings = dacSettings(dacData[...,0], dacData[...,1], MeV)
        settings = setRange(settings, rangeData[...,0])
    elif dacType == 'FLE':
        settings = dacSettings(dacData[...,2], dacData[...,3], MeV)
        settings = setRange(settings, rangeData[...,1])
    elif dacType == 'FHE':
        settings = dacSettings(dacData[...,4], dacData[...,5], MeV)
        settings = setRange(settings, rangeData[...,2]) 
    else:
        settings = uldSettings(uldData[...,0], uldData[...,1], uldData[...,2], (MeV / 100)) 
        settings = setRange(settings, Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW,
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
        hegain = gain
    elif dacType == 'ULD':
        legain = None
        hegain = None
    else:
        legain = gain
        hegain = None
        
    log.info("Creating file %s", outName)
    fio = calDacXML.calDacXML(outName, DAC_MAP[dacType], calDacXML.MODE_CREATE)
    fio.write(settings, filename = outName, leGain = legain, heGain = hegain, energy = energy, 
        engfilename = dacSlopesName, adcmargin = adcmargin, tems = towers)
    fio.close() 
  
 
    sys.exit(0)
     

   
