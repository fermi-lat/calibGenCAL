"""
Tool to produce CAL DAC XML calibration data file.  The command line is:

adccompress [-V] [-L <log_file>] <cfg_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    -L <log_file>   = save console output to log text file
    <cfg_file>      = The application configuration file to use.
    <out_xml_file>  = The compressed CAL DAC calibration XML file to output.
"""


__facility__  = "Offline"
__abstract__  = "Tool to produce CAL DAC XML calibration data file"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/04/14 15:53:07 $"
__version__   = "$Revision: 1.24 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import logging
import ConfigParser
import getopt

import Numeric
import mpfit

import calDacXML
import calFitsXML
import calConstant




# relgain table indicies


LE_REF_GAIN = 5
HE_REF_GAIN = 15
MUON_GAIN   = 8






class inputFile:
    """
    Represents one input XML file.
    """
    
    def __init__(self, srcTwr, destTwr, name):
        """
        inputFile constructor

        Param: srcTwr The data source tower number (0 - 15).
        Param: destTwr The data destination tower number (0 - 15).
        Param: name The input file name
        """
        
        self.srcTwr = srcTwr
        self.destTwr = destTwr
        self.name = name
	


def residuals(p, y, x, fjac = None):

    err = y - ((p[0] * x) + p[1])
    return (0, err)
    
    
    

D0    = Numeric.arange(0.0, 64.0, 1.0)
PI    = {'fixed':0, 'limited':(0,0), 'mpprint':0}
PINFO = [PI, PI]
P0    = (20.0, -300.0)    
    

def fitDAC(fineThresholds, coarseThresholds, adcs0, adcs1):
    """
    Do linear fit of ADC/DAC curve in ADC range
    Param: fineThresholds - ADC thresholds for DAC FINE range
    Param: coarseThresholds - ADC thresholds for DAC COARSE range
    Param: adcs0 - Lower energy limit converted to ADC units
    Param: adcs1 - Upper energy limit converted to ADC units
    Returns: Tuple: 0 = Array of shape (16,8,2,12,2) where the last dimension holds the linear
                        fit parameters for each channel.
                    1 = Array of shape (16,8,2,12) filled in with FINE/COARSE selection per channel    
    """
               
    # array to hold fit parameters
    
    mevs = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 2), Numeric.Float32)
        
    # array to hold range info 
    
    ranges = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), Numeric.Int)    
    
    adcs0 = adcs0[...,Numeric.NewAxis]
    adcs1 = adcs1[...,Numeric.NewAxis]
    
    # compare upper and lower limits against FINE range thresholds 
    
    q = (fineThresholds > adcs0) & (fineThresholds < adcs1)
    
    for tem in range(calConstant.NUM_TEM):
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                
                    qx = q[tem,row,end,fe,:]
                                    
                    # if there are not enough points in the FINE range or high point is out of range, 
                    # try the COARSE range
                
                    if len(Numeric.nonzero(qx)) < 3 or qx[-1]:
                    
                        qx[...] = \
                            (coarseThresholds[tem,row,end,fe,:] > adcs0[tem,row,end,fe,:]) & \
                            (coarseThresholds[tem,row,end,fe,:] < adcs1[tem,row,end,fe,:])
                        
                        tholds = coarseThresholds[tem,row,end,fe,:]
                        ranges[tem,row,end,fe] = calConstant.CDAC_COARSE
                        
                    else:
                    
                        tholds = fineThresholds[tem,row,end,fe,:]
                        ranges[tem,row,end,fe] = calConstant.CDAC_FINE     
                        
                    d = Numeric.compress(qx, D0)
                    a = Numeric.compress(qx, tholds)
                    fkw = {'x' : d, 'y' : a}
                    
                    try:
                        fit = mpfit.mpfit(residuals, P0, functkw = fkw, parinfo = PINFO, quiet = True) 
                    except ValueError, e:
                        log.error("mpfit excep on T%d,%s%s,%d: %s,%s,%s", tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe, e, d, a)
                    if fit.status <= 0:
                        log.error("mpfit error on T%d,%s%s,%d: %s,%s,%s", tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe, fit.errmsg, d, a)
                         
                    mevs[tem,row,end,fe,0] = fit.params[0]
                    mevs[tem,row,end,fe,1] = fit.params[1] 
                    
    return (mevs, ranges)
                        
     
                        
def fitULD(tholds):
    """
    Do linear fit of ADC/ULD curve
    Param: tholds - ADC thresholds for ULD COARSE range
    Returns: Tuple: Array of shape (3,16,8,2,12,3) where the last dimension holds the linear
                    fit parameters for each channel.
    """
               
    # array to hold fit parameters
    
    mevs = Numeric.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 3), Numeric.Float32)
    
    for erng in range(3):
        for tem in range(calConstant.NUM_TEM):
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                                    
                        # remove saturation plateau before fit
                       
                        th = tholds[erng,tem,row,end,fe,:] 
                        sat = th[-1]
                        q =  th < sat
                        a = Numeric.compress(q, th)
                        d = Numeric.compress(q, D0)    
                        fkw = {'x' : d, 'y' : a}
                    
                        try:
                            fit = mpfit.mpfit(residuals, P0, functkw = fkw, parinfo = PINFO, quiet = True) 
                        except ValueError, e:
                            log.error("mpfit excep on %s,T%d,%s%s,%d: %s,%s,%s", calConstant.CRNG[erng],
                                tem, calConstant.CROW[row], calConstant.CPM[end], fe, e, d, a)
                            
                        if fit.status <= 0:
                            log.error("mpfit error on %s,T%d,%s%s,%d: %s,%s,%s", calConstant.CRNG[erng],
                                tem, calConstant.CROW[row], calConstant.CPM[end], fe, fit.errmsg, d, a)
                         
                        mevs[erng,tem,row,end,fe,0] = fit.params[0]
                        mevs[erng,tem,row,end,fe,1] = fit.params[1] 
                        mevs[erng,tem,row,end,fe,2] = sat
                    
    return mevs
    
    

##################################################################################        



if __name__ == '__main__':


    usage = "adccompress [-V] <cfg_file> <out_xml_file>"


    # setup logger

    logging.basicConfig()
    log = logging.getLogger('adccompress')
    log.setLevel(logging.INFO) 

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V-L:")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
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
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    configName = args[0]
    calibName = args[1]


    # read config file settings

    log.info("Reading file %s", configName)
    configFile = ConfigParser.SafeConfigParser()
    configFile.read(configName)
    sections = configFile.sections()
    if len(sections) == 0:
        log.error("Config file %s missing or empty" % configName)
        sys.exit(1)
        
    # get energy limits for fits
    
    lacLow = None
    lacHigh = None
    fleLow = None
    fleHigh = None
    fheLow = None
    fheHigh = None
    
    if 'energies' not in sections:
        log.error("Config file %s missing [energies] section", configName)
        sys.exit(1)
        
    options = configFile.options('energies')
    for opt in options:
    
        if opt == 'lac_low':
            lacLow = float(configFile.get('energies', opt))
        elif opt == 'lac_high':
            lacHigh = float(configFile.get('energies', opt))
        elif opt == 'fle_low':
            fleLow = float(configFile.get('energies', opt)) 
        elif opt == 'fle_high':
            fleHigh = float(configFile.get('energies', opt)) 
        elif opt == 'fhe_low':
            fheLow = float(configFile.get('energies', opt))  
        elif opt == 'fhe_high':
            fheHigh = float(configFile.get('energies', opt))
            
    if lacLow is None:
        log.error("Config file %s missing required [energies]:lac_low option", configName)
        sys.exit(1)
    else:
        log.debug("Using LAC low energy limit %f MeV", lacLow)
    
    if lacHigh is None:
        log.error("Config file %s missing required [energies]:lac_high option", configName)
        sys.exit(1)
    else:
        log.debug("Using LAC high energy limit %f MeV", lacHigh)
        
    if fleLow is None:
        log.error("Config file %s missing required [energies]:fle_low option", configName)
        sys.exit(1)
    else:
        log.debug("Using FLE low energy limit %f MeV", fleLow)
        
    if fleHigh is None:
        log.error("Config file %s missing required [energies]:fle_high option", configName)
        sys.exit(1)
    else:
        log.debug("Using FLE high energy limit %f MeV", fleHigh)
        
    if fheLow is None:
        log.error("Config file %s missing required [energies]:fhe_low option", configName)
        sys.exit(1)
    else:
        log.debug("Using FHE low energy limit %f MeV", fheLow)
        
    if fheHigh is None:
        log.error("Config file %s missing required [energies]:fhe_high option", configName)
        sys.exit(1)
    else:
        log.debug("Using FHE high energy limit %f MeV", fheHigh)                                                    

    # get DAC/ADC characterization file names

    if 'adcfiles' not in sections:
        log.error("Config file %s missing [adcfiles] section" % configName)
        sys.exit(1)

    uldAdcFiles = []
    lacAdcFiles = []
    fleAdcFiles = []
    fheAdcFiles = []
    biasAdcFiles = []

    options = configFile.options('adcfiles')
    for opt in options:
        
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        
        if optList[0] == 'uld2adc':
            fList = uldAdcFiles
        elif optList[0] == 'lac2adc':
            fList = lacAdcFiles
        elif optList[0] == 'fle2adc':
            fList = fleAdcFiles
        elif optList[0] == 'fhe2adc':
            fList = fheAdcFiles
        elif optList[0] == 'bias':
            fList = biasAdcFiles
        else:
            continue

        destTwr = int(optList[1])        
        if destTwr < 0 or destTwr > 15:
            log.error("Index for [adcfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
            
        value = configFile.get('adcfiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [adcfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)    
        inFile = inputFile(srcTwr, destTwr, name)
        fList.append(inFile)
        
        log.debug('Adding %s file %s to input as tower %d (from %d)', optList[0], name,
                  destTwr, srcTwr)
                  
    # get gain file names

    if 'gainfiles' not in sections:
        log.error("Config file %s missing [gainfiles] section" % configName)
        sys.exit(1)

    adc2nrgFiles = []
    relgainFiles = []

    options = configFile.options('gainfiles')
    for opt in options:
        
        optList = opt.split('_')
        if len(optList) != 2:
            continue
        
        if optList[0] == 'adc2nrg':
            fList = adc2nrgFiles
        elif optList[0] == 'relgain':
            fList = relgainFiles
        else:
            continue

        destTwr = int(optList[1])        
        if destTwr < 0 or destTwr > 15:
            log.error("Index for [gainfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)
            
        value = configFile.get('gainfiles', opt)
        nameList = value.split(',')
        nameLen = len(nameList)
        if nameLen == 2:
            name = nameList[0]
            srcTwr = int(nameList[1])
        else:
            log.error("Incorrect option format %s", value)
            sys.exit(1)
        if srcTwr < 0 or srcTwr > 15:
            log.error("Src index for [gainfiles] option %s out of range (0 - 15)", opt)
            sys.exit(1)    
        inFile = inputFile(srcTwr, destTwr, name)
        fList.append(inFile)
        
        log.debug('Adding %s file %s to input as tower %d (from %d)', optList[0], name,
                  destTwr, srcTwr)
                  

    # create empty ADC data arrays

    lacAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    fleAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    fheAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)
    biasAdcData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                 calConstant.NUM_FE, 2), Numeric.Float32)
    relgainData = Numeric.zeros((9, calConstant.NUM_RNG, calConstant.NUM_TEM, calConstant.NUM_ROW,
                              calConstant.NUM_END, calConstant.NUM_FE), Numeric.Float32)
    adc2nrgData = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 2), Numeric.Float32)
    uldAdcData = Numeric.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                                calConstant.NUM_FE, 128), Numeric.Float32)                            
                                
    # read LAC/ADC characterization file
    
    for f in lacAdcFiles:    

        log.info("Reading file %s", f.name)
        lacAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = lacAdcFile.info()
        if i['TTYPE1'] != 'log_acpt':
            log.error("File %s not a LAC ADC file", f.name)
            sys.exit(1)
        if i['ERNG'] != 'LEX8':
            log.error("Only LEX8 energy range is supported for LAC DAC")
            sys.exit(1)
        if int(i['LREFGAIN']) != LE_REF_GAIN:
            log.error("Only LE gain %d supported for LAC DAC.", LE_REF_GAIN)
            sys.exit(1)
        twrs = lacAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = lacAdcFile.read()
        lacAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        lacAdcFile.close()


    # read FLE/ADC characterization files

    for f in fleAdcFiles:
        
        log.info("Reading file %s", f.name)
        fleAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = fleAdcFile.info()
        if i['TTYPE1'] != 'fle_dac':
            log.error("File %s not a FLE ADC file", f.name)
            sys.exit(1)
        if i['ERNG'] != 'LEX1':
            log.error("Only LEX1 energy ranges supported for FLE DAC")
            sys.exit(1)
        if int(i['LREFGAIN']) != LE_REF_GAIN:
            log.error("Only LE gain %d supported for FLE DAC.", LE_REF_GAIN)
            sys.exit(1)
        twrs = fleAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = fleAdcFile.read()
        fleAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fleAdcFile.close()
        
        
    # read FHE/ADC characterization files

    for f in fheAdcFiles:    

        log.info("Reading file %s", f.name)
        fheAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = fheAdcFile.info()
        if i['TTYPE1'] != 'fhe_dac':
            log.error("File %s not a FHE ADC file", f.name)
            sys.exit(1)
        if i['ERNG'] != 'HEX8':
            log.error("Only HEX8 energy range is supported for FHE DAC")
            sys.exit(1)
        if int(i['HREFGAIN']) != HE_REF_GAIN:
            log.error("Only HE gain %d supported for FHE DAC.", HE_REF_GAIN)
            sys.exit(1)
        twrs = fheAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = fheAdcFile.read()
        fheAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        fheAdcFile.close()

    # read bias correction files

    for f in biasAdcFiles:

        log.info("Reading file %s", f.name)
        biasFile = calDacXML.calEnergyXML(f.name, 'thrBias')
        twrs = biasFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        adcData = biasFile.read()
        biasAdcData[f.destTwr,...] = adcData[f.srcTwr,...]
        biasFile.close()
        
    # read relgain files
    
    for f in relgainFiles:
    
        log.info("Reading file %s", f.name)
        relgainFile = calFitsXML.calFitsXML(fileName = f.name)
        twrs = relgainFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        gainData = relgainFile.read()
        relgainData[:,:,f.destTwr,...] = gainData[:,:,f.srcTwr,...]
        relgainFile.close()


    # read ADC/energy files
    
    for f in adc2nrgFiles:
    
        log.info("Reading file %s", f.name)
        adc2nrgFile = calDacXML.calEnergyXML(f.name, 'adc2nrg')
        twrs = adc2nrgFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        engData = adc2nrgFile.read()    
        adc2nrgData[f.destTwr, ...] = engData[f.srcTwr, ...]
        adc2nrgFile.close()
        
    # read ULD/ADC characterization files

    for f in uldAdcFiles:        
    
        log.info("Reading file %s", f.name)
        uldAdcFile = calFitsXML.calFitsXML(fileName = f.name, mode = calFitsXML.MODE_READONLY)
        i = uldAdcFile.info()
        if i['TTYPE1'] != 'rng_uld_dac':
            log.error("File %s not a ULD ADC file", f.name)
            sys.exit(1)
        twrs = uldAdcFile.getTowers()
        if f.srcTwr not in twrs:
            log.error("Src twr %d data not found in file %s", f.srcTwr, f.name)
            sys.exit(1)
        #if int(i['HREFGAIN']) != HE_REF_GAIN or int(i['LREFGAIN']) != LE_REF_GAIN:
        #    log.error("Only gains %d, %d not supported for ULD DAC.", 
        #        LE_REF_GAIN, HE_REF_GAIN)
        #    sys.exit(1)
        adcData = uldAdcFile.read()
        uldAdcData[:,f.destTwr,...] = adcData[:,f.srcTwr,...]
        uldAdcFile.close()
        
        
    #---------------------------- do FLE processing ---------------------------------------------
    
    # split into fine and coarse ranges
    
    fineThresholds = fleAdcData[...,:64]
    log.debug("FLE fineThresholds[0,0,0,0,:]:%s", str(fineThresholds[0,0,0,0,:]))
    coarseThresholds = fleAdcData[...,64:]
    log.debug("FLE coarseThresholds[0,0,0,0,:]:%s", str(coarseThresholds[0,0,0,0,:]))
    
    # convert energy to ADC threshold
    
    adcs0 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), Numeric.Float32)
    adcs1 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
       calConstant.NUM_FE), Numeric.Float32)
    adcs0 = adcs0 * fleLow
    adcs1 = adcs1 * fleHigh
    log.debug("FLE adcs[0,0,0,0]:%.3f %.3f", adcs0[0,0,0,0], adcs1[0,0,0,0])    
            
    # energy->ADC conversion
    
    adcs0 /= adc2nrgData[...,0]
    adcs1 /= adc2nrgData[...,0]
    log.debug("FLE adcs0[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", adcs0[0,0,0,0], adcs1[0,0,0,0], 
        adc2nrgData[0,0,0,0,0])
            
    # trigger bias correction
    
    adcs0 -= biasAdcData[...,0]
    adcs1 -= biasAdcData[...,0]
    log.debug("FLE adcs0[0,0,0,0]:%.3f %.3f bias[0,0,0,0,0]:%.3f", adcs0[0,0,0,0], adcs1[0,0,0,0], 
        biasAdcData[0,0,0,0,0])
            
    # energy range scaling
    
    rngScale = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), Numeric.Float32)
    rngScale = rngScale * 9.0    
        
    adcs0 /= rngScale
    adcs1 /= rngScale
    log.debug("FLE adcs[0,0,0,0]:%.3f %.3f rngScale[0,0,0,0]:%.3f", adcs0[0,0,0,0],
        adcs1[0,0,0,0], rngScale[0,0,0,0])
    
    # get linear fit parameters
    
    log.info("Compressing FLE")
    (mevs, ranges) = fitDAC(fineThresholds, coarseThresholds, adcs0, adcs1)                       
    
    # convert DAC/ADC slopes to DAC/energy slopes
         
    log.debug("FLE mevs[0,0,0,0]:%.3f %.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1])
    
    # energy range scaling
    
    mevs *= rngScale[...,Numeric.NewAxis].astype(Numeric.Float32)
    log.debug("FLE mevs[0,0,0,0]:%.3f %.3f rngScale[0,0,0,0]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1],
        rngScale[0,0,0,0]) 
        
    # trigger bias correction
    
    mevs[...,1] += biasAdcData[...,0]
    log.debug("FLE mevs[0,0,0,0]:%.3f %.3f bias[0,0,0,0,0]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1], 
        biasAdcData[0,0,0,0,0])    
    
    # ADC->energy conversion
    
    mevs *= adc2nrgData[...,0,Numeric.NewAxis]
    
    log.debug("FLE mevs[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1],
        adc2nrgData[0,0,0,0,0]) 
    log.debug("FLE ranges[0,0,0,0]: %s", calConstant.CDAC[ranges[0,0,0,0]])      
    
    
    #---------------------------- do FHE processing ---------------------------------------------
    
    # split into fine and coarse ranges
    
    fineThresholds = fheAdcData[...,:64]
    log.debug("FHE fineThresholds[0,0,0,0,:]:%s", str(fineThresholds[0,0,0,0,:]))
    coarseThresholds = fheAdcData[...,64:]
    log.debug("FHE coarseThresholds[0,0,0,0,:]:%s", str(coarseThresholds[0,0,0,0,:]))
    
    # convert energy to ADC threshold
    
    adcs0 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs1 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs0 = adcs0 * fheLow
    adcs1 = adcs1 * fheHigh
    log.debug("FHE adcs[0,0,0,0]:%.3f %.3f", adcs0[0,0,0,0], adcs1[0,0,0,0])
            
    # relative gain factor
    
    adcs0 /= relgainData[MUON_GAIN,calConstant.CRNG_HEX8,...]
    adcs1 /= relgainData[MUON_GAIN,calConstant.CRNG_HEX8,...]
        
    log.debug("FHE adcs[0,0,0,0]:%.3f %.3f relgain[0,0,0,0]:%.3f", adcs0[0,0,0,0], adcs1[0,0,0,0], 
            (1.0 / relgainData[MUON_GAIN,calConstant.CRNG_HEX8,0,0,0,0]))

    # energy->ADC conversion
    
    adcs0 /= adc2nrgData[...,1]
    adcs1 /= adc2nrgData[...,1]
    log.debug("FHE adcs0[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,1]:%.3f", adcs0[0,0,0,0],
            adcs1[0,0,0,0], adc2nrgData[0,0,0,0,1])
            
    # trigger bias correction
    
    adcs0 -= biasAdcData[...,1]
    adcs1 -= biasAdcData[...,1]
    log.debug("FHE adcs0[0,0,0,0]:%.3f %.3f bias[0,0,0,0,0]:%.3f", adcs0[0,0,0,0],
            adcs1[0,0,0,0], biasAdcData[0,0,0,0,1])
            
    # get linear fit parameters
    
    log.info("Compressing FHE")
    (mevs, ranges) = fitDAC(fineThresholds, coarseThresholds, adcs0, adcs1)  
                        
    # convert DAC/ADC slopes to DAC/energy slopes
         
    log.debug("FHE mevs[0,0,0,0]:%.3f %.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1])
    
    # trigger bias correction
    
    mevs[...,1] += biasAdcData[...,1]
    log.debug("FHE mevs[0,0,0,0]:%.3f %.3f bias[0,0,0,0,1]:%.3f", mevs[0,0,0,0,0],
            mevs[0,0,0,0,1], biasAdcData[0,0,0,0,1])    
    
    # ADC->energy conversion
    
    mevs *= adc2nrgData[...,1,Numeric.NewAxis]
    log.debug("FHE mevs[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,1]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1],
        adc2nrgData[0,0,0,0,1])  
    
    # relative gain factor
            
    mevs *= relgainData[MUON_GAIN,calConstant.CRNG_HEX8,...,Numeric.NewAxis] 
                
    log.debug("FHE mevs[0,0,0,0]:%.3f %.3f relgain[0,0,0,0]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1], 
        relgainData[MUON_GAIN,calConstant.CRNG_HEX8,0,0,0,0])  
    log.debug("FHE ranges[0,0,0,0]: %s", calConstant.CDAC[ranges[0,0,0,0]])    
   
    #---------------------------- do LAC processing ---------------------------------------------
    
    # split into fine and coarse ranges
    
    fineThresholds = lacAdcData[...,:64]
    log.debug("LAC fineThresholds[0,0,0,0,:]:%s", str(fineThresholds[0,0,0,0,:]))
    coarseThresholds = lacAdcData[...,64:]
    log.debug("LAC coarseThresholds[0,0,0,0,:]:%s", str(coarseThresholds[0,0,0,0,:]))
    
    # convert energy to ADC threshold
    
    adcs0 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs1 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs0 = adcs0 * lacLow
    adcs1 = adcs1 * lacHigh
    log.debug("LAC adcs[0,0,0,0]:%.3f %.3f", adcs0[0,0,0,0], adcs1[0,0,0,0])
            
    # energy->ADC conversion
    
    adcs0 /= adc2nrgData[...,0]
    adcs1 /= adc2nrgData[...,0]
    log.debug("LAC adcs0[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", adcs0[0,0,0,0],
            adcs1[0,0,0,0], adc2nrgData[0,0,0,0,0])
            
    # get linear fit parameters
    
    log.info("Compressing LAC")
    (mevs, ranges) = fitDAC(fineThresholds, coarseThresholds, adcs0, adcs1)
    
    # convert DAC/ADC slopes to DAC/energy slopes
         
    log.debug("LAC mevs[0,0,0,0]:%.3f %.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1])
    
    # ADC->energy conversion
    
    mevs *= adc2nrgData[...,0,Numeric.NewAxis]
    log.debug("LAC mevs[0,0,0,0]:%.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", mevs[0,0,0,0,0], mevs[0,0,0,0,1],
        adc2nrgData[0,0,0,0,0])  
    log.debug("LAC ranges[0,0,0,0]: %s", calConstant.CDAC[ranges[0,0,0,0]]) 
    
    #---------------------------- do ULD processing ---------------------------------------------
    
    # split into fine and coarse ranges
    
    coarseThresholds = uldAdcData[...,64:]
    log.debug("ULD coarseThresholds[0,0,0,0,0,:]:%s", str(coarseThresholds[0,0,0,0,0,:]))
    log.debug("ULD coarseThresholds[1,0,0,0,0,:]:%s", str(coarseThresholds[1,0,0,0,0,:]))
    log.debug("ULD coarseThresholds[2,0,0,0,0,:]:%s", str(coarseThresholds[2,0,0,0,0,:]))
            
    # get linear fit parameters
    
    log.info("Compressing ULD")
    adcs0 = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs1 = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
            calConstant.NUM_FE), Numeric.Float32)
    adcs1 = adcs1 * 4096.0
    
    mevData = Numeric.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE, 3), Numeric.Float32)
        
    mevs = fitULD(coarseThresholds)  
                        
    # convert DAC/ADC slopes to DAC/energy slopes
         
    log.debug("ULD mevs[0,0,0,0,0]:%.3f %.3f %.3f", mevs[0,0,0,0,0,0], 
        mevs[0,0,0,0,0,1], mevs[0,0,0,0,0,2])
    log.debug("ULD mevs[1,0,0,0,0]:%.3f %.3f %.3f", mevs[1,0,0,0,0,0], 
        mevs[1,0,0,0,0,1], mevs[1,0,0,0,0,2])    
    log.debug("ULD mevs[2,0,0,0,0]:%.3f %.3f %.3f", mevs[2,0,0,0,0,0], 
        mevs[2,0,0,0,0,1], mevs[2,0,0,0,0,2])
    
    # energy range scaling
    
    rngScale = Numeric.ones((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
        calConstant.NUM_FE), Numeric.Float32)
    rngScale = rngScale * 9.0
    mevs[calConstant.CRNG_LEX1,...] *= rngScale[...,Numeric.NewAxis].astype(Numeric.Float32)   
    log.debug("ULD mevs[1,0,0,0,0]:%.3f %.3f %.3f rngScale[0,0,0,0]:%.3f", 
        mevs[1,0,0,0,0,0], mevs[1,0,0,0,0,1], mevs[1,0,0,0,0,2],
        rngScale[0,0,0,0])
        
    # ADC->energy conversion
    
    mevs[calConstant.CRNG_LEX8,...] *= adc2nrgData[...,0,Numeric.NewAxis]
    mevs[calConstant.CRNG_LEX1,...] *= adc2nrgData[...,0,Numeric.NewAxis]
    mevs[calConstant.CRNG_HEX8,...] *= adc2nrgData[...,1,Numeric.NewAxis]
    log.debug("ULD mevs[0,0,0,0,0]:%.3f %.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", 
        mevs[0,0,0,0,0,0], mevs[0,0,0,0,0,1], mevs[0,0,0,0,0,2],
        adc2nrgData[0,0,0,0,0]) 
    log.debug("ULD mevs[1,0,0,0,0]:%.3f %.3f %.3f adc2nrg[0,0,0,0,0]:%.3f", 
        mevs[1,0,0,0,0,0], mevs[1,0,0,0,0,1], mevs[1,0,0,0,0,2],
        adc2nrgData[0,0,0,0,0])    
    log.debug("ULD mevs[2,0,0,0,0]:%.3f %.3f %.3f adc2nrg[0,0,0,0,1]:%.3f", 
        mevs[2,0,0,0,0,0], mevs[2,0,0,0,0,1], mevs[2,0,0,0,0,2],
        adc2nrgData[0,0,0,0,1]) 
       
    # relative gain factor
            
    mevs[calConstant.CRNG_HEX8,...] *= relgainData[MUON_GAIN,calConstant.CRNG_HEX8,...,Numeric.NewAxis] 
                
    log.debug("ULD mevs[2,0,0,0,0]:%.3f %.3f %.3f relgain[0,0,0,0]:%.3f", 
        mevs[2,0,0,0,0,0], mevs[2,0,0,0,0,1], mevs[2,0,0,0,0,2],
        relgainData[MUON_GAIN,calConstant.CRNG_HEX8,0,0,0,0]) 
             
    sys.exit(0)


