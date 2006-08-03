"""
Validate CAL DAC (FHE, FLE, or LAC) settings XML files.  The command line is:

valDACsettings [-V] [-R <root_file>] [-L <log_file>] FLE|FHE|LAC <MeV> <dac_slopes_file> <dac_xml_file>

where:
    -R <root_file>     = output validation diagnostics in ROOT file
    -L <log_file>      = save console output to log text file
    -V                 = verbose; turn on debug output
    FLE|FHE|LAC        = DAC type to validate
    <MeV>              = The threshold energy in MeV units.
    <dac_slopes_file>  = The CAL_DacSlopes calibration file.
    <dac_xml_file>     = The DAC settings XML file to validate.
"""



__facility__    = "Offline"
__abstract__    = "Validate CAL DAC settings XML files."
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/07/03 19:28:23 $"
__version__     = "$Revision: 1.10 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



import sys, os, time
import logging
import getopt

import Numeric

import calCalibXML
import calDacXML
import calConstant




def rootHists(engData, fileName):

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



def engVal(errData):

    valStatus = 0
    
    # check for ADC->energy conversion error
    
    for tem in twrs:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):

                    err = errData[tem, row, end, fe]                
                    if err > warnLimit:
                        if err > errLimit:
                            log.error('err %0.2f > %0.2f for %d,%s%s,%d', err, errLimit, tem, calConstant.CROW[row],
                                  calConstant.CPM[end], fe)
                            valStatus = 1
                        else:
                            log.warning('err %0.2f > %0.2f for %d,%s%s,%d', err, warnLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe)
    return valStatus			    
                        



if __name__ == '__main__':
    
    usage = "valDACsettings [-V] [-R <root_file>] [-L <log_file>] FLE|FHE|LAC <MeV> <dac_slopes_file> <dac_xml_file>"

    rootOutput = False
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('dacVal')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V")
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
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('Deleting old log file %s', o[1])
                os.remove(o[1])
            hdl = logging.FileHandler(o[1])
            fmt = logging.Formatter('%(levelname)s %(message)s')
            hdl.setFormatter(fmt)
            log.addHandler(hdl)
        
    args = opts[1]
    if len(args) != 4:
        log.error(usage)
        sys.exit(1)

    dacType = args[0]
    MeV = float(args[1])
    slopesName = args[2]
    dacName = args[3]
    
    # set limits base on DAC type 
    
    if dacType == 'FLE':
        type = 'fle_dac'
        warnLimit = 5.0
        errLimit = 10.0
    elif dacType == 'FHE':
        type = 'fhe_dac'
        warnLimit = 50.0
        errLimit = 100.0
    elif dacType == 'LAC':
        type = 'log_acpt'
        warnLimit = 0.5
        errLimit = 1.0
    else:
        log.error("DAC type %s not supported", dacType)
        sys.exit(1)

    log.debug('Using error limit %f', errLimit)
    log.debug('Using warning limit %f', warnLimit)
    log.debug('Using DAC type %s', type)
    log.debug('Using threshold %f MeV', MeV)

    # read dacSlopes gain file

    log.info("Reading file %s", slopesName)
    slopeFile = calCalibXML.calDacSlopesCalibXML(slopesName)
    (dacSlope, uldSlope, rangeData) = slopeFile.read()
    towers = slopeFile.getTowers()
    slopeFile.close()

    # read DAC settings file

    log.info('Reading file %s', dacName)
    fio = calDacXML.calDacXML(dacName, type)
    twrs = set(fio.getTowers())
    if not twrs.issubset(towers):
        log.error("%s data not found in file %s", twrs, dacName)
        sys.exit(1)
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
        
    dacData = Numeric.where(ranges, (dacData - 64), dacData)    
    
    # convert to MeV
    
    eng = (slope * dacData) + offset
    
    # calculate errors

    err = abs(eng - MeV)

    # do validation

    valStatus = engVal(err)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(eng, dacName)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, dacName)
    sys.exit(valStatus)    
     

   
