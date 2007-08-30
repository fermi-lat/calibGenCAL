"""
Validate CAL DAC settings XML files.  The command line is:

valDACsettings [-V] [-r] [-R <root_file>] [-L <log_file>] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_file> <dac_xml_file>

where:
    -r                 = generate ROOT output with default name
    -R <root_file>     = output validation diagnostics in ROOT file
    -L <log_file>      = save console output to log text file
    -V                 = verbose; turn on debug output
    FLE|FHE|LAC        = DAC type to validate
    <MeV>              = The threshold energy in MeV units (FLE, FHE, LAC).
    <margin>           = The ULD saturation margin percentage (ULD).
    <dac_slopes_file>  = The CAL_DacSlopes calibration file.
    <dac_xml_file>     = The DAC settings XML file to validate.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/10/16 15:46:32 $"
__version__     = "$Revision: 1.6 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



import sys, os
import logging
import getopt

import Numeric

import calCalibXML
import calDacXML
import calConstant



# validation limits

uldWarnLimit = 0.005
uldErrLimit  = 0.010




def rootHistsDAC(engData, fileName):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary"      
    hs = ROOT.TH1F(hName, "DAC_Val_%s: %s" % (dacType, fileName), 100, MeV - (errLimit * 2), MeV + (errLimit * 2))
    axis = hs.GetXaxis()
    axis.SetTitle('Threshold Energy (MeV)')
    axis.CenterTitle()
    axis = hs.GetYaxis()
    axis.SetTitle('Counts')
    axis.CenterTitle()
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    e = engData[tem, row, end, fe]
                    hs.Fill(e)
                    
    hs.Draw()
    cs.Update()
    cs.Write()



def engValDAC(dacData, errData):

    valStatus = 0
    
    # sanity check on DAC settings
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                
                    dac = dacData[tem, row, end, fe]
                    
                    if dac > 63:
                        log.error('setting %d > 63 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                        valStatus = 1    
                    
                    elif dac == 63:
                        log.warning('setting %d == 63 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                            
                    elif dac == 0:
                        log.error('setting %d == 0 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                        valStatus = 1        
                            
   
    # check for ADC->energy conversion error
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    err = errData[tem, row, end, fe]                
                    if err > warnLimit:
                        if err > errLimit:
                            log.error('err %0.2f > %0.2f for T%d,%s%s,%d', err, errLimit, tem, calConstant.CROW[row],
                                  calConstant.CPM[end], fe)
                            valStatus = 1
                        else:
                            log.warning('err %0.2f > %0.2f for T%d,%s%s,%d', err, warnLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe)
    return valStatus			    
                        




def engValULD(dacData, engData, saturation):

    valStatus = 0
    
    # sanity check on DAC settings
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                
                    dac = dacData[tem, row, end, fe]
                    
                    if dac > 63:
                        log.error('setting %d > 63 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                        valStatus = 1    
                    
                    elif dac == 63:
                        log.warning('setting %d == 63 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                            
                    elif dac == 0:
                        log.error('setting %d == 0 for T%d,%s%s,%d', dac, tem, calConstant.CROW[row],
                            calConstant.CPM[end], fe)
                        valStatus = 1
    
    # check ULD threshold verses saturation to see if margin is kept
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(3):
                    
                        eng = engData[erng, tem, row, end, fe]
                        sat = saturation[erng, tem, row, end, fe]
                        margin = (sat - eng) / sat
                        
                        if margin < MeV:
                        
                            err = abs(MeV - margin)                            
                        
                            if err > uldWarnLimit:
                                if err > uldErrLimit:
                                    log.error("err %0.3f > %0.3f for T%d,%s%s,%d,%s", err, uldErrLimit, tem, calConstant.CROW[row],
                                        calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    valStatus = 1
                                else:
                                    log.warning("err %0.3f > %0.3f for T%d,%s%s,%d,%s", err, uldWarnLimit, tem, calConstant.CROW[row],
                                        calConstant.CPM[end], fe, calConstant.CRNG[erng]) 
                                    
    return valStatus                                                   





if __name__ == '__main__':
    
    usage = \
        "valDACsettings [-V] [-r] [-R <root_file>] [-L <log_file>] FLE|FHE|LAC|ULD <MeV | margin> <dac_slopes_file> <dac_xml_file>"

    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('valDACsettings')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        elif o[0] == '-r':
            rootName = None
            rootOutput = True
        elif o[0] == '-L':
            logName = o[1]
           
        
    args = opts[1]
    if len(args) != 4:
        log.error(usage)
        sys.exit(1)

    dacType = args[0]
    MeV = float(args[1])
    slopesName = args[2]
    dacName = args[3]
    
    ext = os.path.splitext(dacName)
    if rootOutput and rootName is None:
        rootName = "%s.val.root" % ext[0]
    if logName is None:
        logName = "%s.val.log" % ext[0]

    if os.path.exists(logName):
        log.debug('Removing old log file %s', logName)
        os.remove(logName)

    hdl = logging.FileHandler(logName)
    fmt = logging.Formatter('%(levelname)s %(message)s')
    hdl.setFormatter(fmt)
    log.addHandler(hdl)
    
    # set limits base on DAC type 
    
    if dacType == 'FLE':
        ftype = 'fle_dac'
        warnLimit = 5.0
        errLimit = 10.0
    elif dacType == 'FHE':
        ftype = 'fhe_dac'
        warnLimit = 50.0
        errLimit = 100.0
    elif dacType == 'LAC':
        ftype = 'log_acpt'
        warnLimit = 0.5
        errLimit = 1.0
    elif dacType == 'ULD':
        ftype = 'rng_uld_dac'
        warnLimit = 0.005
        errLimit = 0.010
        MeV /= 100.0
    else:
        log.error("DAC type %s not supported", dacType)
        sys.exit(1)

    log.debug('Using error limit %f', errLimit)
    log.debug('Using warning limit %f', warnLimit)
    log.debug('Using DAC type %s', ftype)
    log.debug('Using threshold %f MeV', MeV)

    # read dacSlopes gain file

    log.info("Reading file %s", slopesName)
    slopeFile = calCalibXML.calDacSlopesCalibXML(slopesName)
    (dacSlope, uldSlope, rangeData) = slopeFile.read()
    towers = slopeFile.getTowers()
    slopeFile.close()

    # read DAC settings file

    log.info('Reading file %s', dacName)
    fio = calDacXML.calSettingsXML(dacName, ftype)
    twrs = set(fio.getTowers())
    if not twrs.issubset(towers):
        log.error("%s data not found in file %s", twrs, dacName)
        sys.exit(1)
    log.debug("settings available for towers: %s", twrs)    
    dacData = fio.read()
    fio.close()
    
    # account for ranges and type
    
    if dacType == 'LAC':
        slope = dacSlope[...,0]
        offset = dacSlope[...,1]
        ranges = rangeData[...,0]
    elif dacType == 'FLE':
        slope = dacSlope[...,2]
        offset = dacSlope[...,3]
        ranges = rangeData[...,1]
    elif dacType == 'FHE':
        slope = dacSlope[...,4]
        offset = dacSlope[...,5]
        ranges = rangeData[...,2]
    elif dacType == 'ULD':
        slope = uldSlope[...,0]
        offset = uldSlope[...,1]
        saturation = uldSlope[...,2]     
        
    origDacData = dacData
    
    # handle FLE, FHE, and LAC cases
        
    if dacType != 'ULD':
                
        dacData = Numeric.where(ranges, (dacData - 64), dacData)    
    
        # convert to MeV
    
        eng = (slope * dacData) + offset
    
        # calculate errors

        err = abs(eng - MeV)

        # do validation

        valStatus = engValDAC(dacData, err) 
        
    # handle ULD case
    
    else:
        
        dacData = dacData - 64
        
        # convert to MeV
        
        eng = (slope * dacData) + offset
 
        # do validation
        
        valStatus = engValULD(dacData, eng, saturation)      
           

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        if dacType != 'ULD':
            rootHistsDAC(eng, dacName)        

        # clean up

        rootFile.Close()
        
        
    # do simple stats 
    
    sf = (origDacData < 64)
    sc = Numeric.logical_not(sf) 
    
    fineCount = Numeric.sum(Numeric.ravel(sf))
    coarseCount = Numeric.sum(Numeric.ravel(sc))
    log.info("FINE count = %d, COARSE count = %d", fineCount, coarseCount)  
    
    data = Numeric.compress(Numeric.ravel(sf), Numeric.ravel(origDacData))
    if len(data) == 0:
        av = 0
        mn = 0
        mx = 0
    else:
        av = Numeric.average(data.astype(Numeric.Float32))
        mn = min(data)
        mx = max(data)
    log.info("FINE setting average = %0.2f", av)
    log.info("FINE setting minimum = %d, maximum = %d", mn, mx)
    
    data = Numeric.compress(Numeric.ravel(sc), Numeric.ravel(origDacData))
    if len(data) == 0:
        av = 0
        mn = 0
        mx = 0
    else:
        av = Numeric.average(data.astype(Numeric.Float32))
        mn = min(data)
        mx = max(data)
    log.info("COARSE setting average = %0.2f", av)
    log.info("COARSE setting minimum = %d, maximum = %d", mn, mx)           

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, dacName)
    sys.exit(valStatus)    
     

   
