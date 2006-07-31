"""
Generate DAC (FHE, FLE, ULD, or LAC) settings XML files using a CAL DacSlopes file as a calibration
reference.  The command line is:

genDACsettings [-V] [-t <timetag>] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_xml_file> [<gain>]

where:
    -V              = verbose; turn on debug output
    -t <timetag>    = id string at front of all output DAC settings filenames (usually 12 digit date string, could be 'latest') 
                      (default = "latest")
    FLE|FHE|LAC|ULD = DAC type to validate
    <MeV | margin>  = For FLE, FHE, LAC: The threshold energy in MeV units.
                      For ULD: The margin as a percentage of the saturation value.
    <dac_slopes_xml_file> = The CAL_DacSlopes calibration file.                     
    <dac_xml_file>  = The DAC settings XML file to validate.
    <gain>          = CAL gain setting DAC's are valid for (needed for FLE, FHE, and LAC only). 
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/07/21 00:39:59 $"
__version__     = "$Revision: 1.2 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



import sys, os, time
import logging
import getopt
import shutil

import Numeric

import calCalibXML
import calDacXML
import calConstant



DAC_MAP = {'FLE' : 'fle_dac', 'FHE' : 'fhe_dac', 'LAC' : 'log_acpt', 'ULD' : 'rng_uld_dac'} 




def dacSettings(slope, offset, energy):

     d = (energy - offset) / slope
     d = Numeric.clip(d, 0, 63)
     return d.astype(Numeric.Int8)
     
     
     
     
def uldSettings(slopes, offsets, sats, margin):

    sats = sats - (sats * margin)
    d = (sats - offsets) / slopes
    d = Numeric.clip(d, 0, 63)
    lex8 = d[calConstant.CRNG_LEX8, ...]
    lex1 = d[calConstant.CRNG_LEX1, ...]  
    hex8 = d[calConstant.CRNG_HEX8, ...]
    s = Numeric.where((lex8 < lex1), lex8, lex1)
    s = Numeric.where((s < hex8), s, hex8)
    return s.astype(Numeric.Int8) 
              
     
     
     
def setRange(dacs, rangeData):

    return Numeric.where(rangeData, (dacs + 64), dacs)
          
  

def gen_basename(timetag, gain, mev, idet, dac_type):
    
    return  "%s_G%d_MeV%d_%s_CAL_%s.xml" % (timetag, int(gain), int(mev), idet, dac_type)
  
  
    
def gen_basename_uld(timetag, adcmargin, idet):

    return "%s_mar%03d_%s_CAL_uld.xml" % (timetag, int(adcmargin), idet)
    
      
     

if __name__ == '__main__':
    
    usage = "genDACsettings [-V] [-t <timetag>] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_xml_file> [<gain>]"

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
    if len(args) < 3:
        log.error(usage)
        sys.exit(1)

    dacType = args[0]
    MeV = float(args[1])
    dacSlopesName = args[2]
    if dacType != 'ULD':
        if len(args) != 4:
            log.error(usage)
            sys.exit(1)
        gain = int(args[3])

    if dacType not in ('FLE', 'FHE', 'LAC', 'ULD'):
        log.error("DAC type %s not supported", dacType)
        sys.exit(1)

    log.debug('Using DAC type %s', dacType)
    if dacType != 'ULD':
        log.debug('Using threshold %f MeV', MeV)
        log.debug('Using gain setting %d', gain)
    else:
        log.debug('Using margin %f %%', MeV) 
           

    # read DAC to energy conversion file
    
    log.info("Reading file %s", dacSlopesName)
    fio = calCalibXML.calDacSlopesCalibXML(dacSlopesName)
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
    
    # create output settings XML files
    
    if timetag is None:
        timestamp = time.strftime('%y%m%d%H%M%S', time.gmtime())
    else:
        timestamp = timetag     
    
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
        
    for tem in range(calConstant.NUM_TEM):
    
        if dacType != 'ULD':
            outName = gen_basename(timestamp, gain, MeV, calConstant.CMOD[tem], dacType.lower())
            copyName = gen_basename('latest', gain, MeV, calConstant.CMOD[tem], dacType.lower())
        else:
            outName = gen_basename_uld(timestamp, int(MeV * 10), calConstant.CMOD[tem])
            copyName = gen_basename_uld('latest', int(MeV * 10), calConstant.CMOD[tem])          
        log.info("Creating file %s", outName)
        fio = calDacXML.calDacXML(outName, DAC_MAP[dacType], calDacXML.MODE_CREATE)
        fio.write(settings, filename = outName, leGain = legain, heGain = hegain, 
            energy = energy, engfilename = dacSlopesName, adcmargin = adcmargin, 
            tems = (tem,))
        fio.close() 
        
        # make copy of with online name format
        
        log.debug("Copying %s to %s", outName, copyName)
        shutil.copy(outName, copyName) 

    sys.exit(0)
     

   
