"""
Validate CAL MuSlope calibration data in XML format.  The command
line is:

muSlopeVal [-V] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL MuSlope calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL MuSlope calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/07/05 18:59:38 $"
__version__   = "$Revision: 1.7 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import math
import getopt
import logging

import Numeric
import MLab

import calCalibXML
import calConstant



# limit values


hex1GainHighWarnLimit = 24.0
hex1GainLowWarnLimit  = 16.0
hex1GainHighErrLimit  = 28.0
hex1GainLowErrLimit   = 12.0

hex8GainHighWarnLimit = 2.8
hex8GainLowWarnLimit  = 1.8
hex8GainHighErrLimit  = 3.0
hex8GainLowErrLimit   = 1.6

lex1GainHighWarnLimit = 0.35
lex1GainLowWarnLimit  = 0.23
lex1GainHighErrLimit  = 0.37
lex1GainLowErrLimit   = 0.21

lex8GainHighWarnLimit = 0.036
lex8GainLowWarnLimit  = 0.024
lex8GainHighErrLimit  = 0.039
lex8GainLowErrLimit   = 0.021

 


def rootHists(slopeData, fileName):

    # create gain summary histogram
    
    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_MuSlope', 'Summary_MuSlope', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()
    cs.SetLogx()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
    for erng in range(calConstant.NUM_RNG):
    
        hName = "h_Summary_MuSlope_%s" % calConstant.CRNG[erng]
        hs = ROOT.TH1F(hName, 'MuSlope_Summary: %s' % fileName, 800, 0.0, 30.0)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()  
        
    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):             
                    for erng in range(calConstant.NUM_RNG):
                        hs = sumHists[erng]
                        slope = slopeData[tem, row, end, fe, erng, 0]
                        hs.Fill(slope)
                        
                        
    hMax = 0
    for erng in range(calConstant.NUM_RNG):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()                    
                        
    for erng in range(calConstant.NUM_RNG):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
            axis = hs.GetXaxis()
            axis.SetTitle('Gain Value (MeV/ADC)')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
            
        hs.SetMaximum(hMax)    
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()   



def calcError(slopeData):

    status = 0
    
    for erng in range(calConstant.NUM_RNG):
    
        # get limit per energy range
    
        if erng == calConstant.CRNG_LEX8:
        
            highErrLimit = lex8GainHighErrLimit
            lowErrLimit = lex8GainLowErrLimit
            highWarnLimit = lex8GainHighWarnLimit
            lowWarnLimit = lex8GainLowWarnLimit
            
        elif erng == calConstant.CRNG_LEX1:
        
            highErrLimit = lex1GainHighErrLimit
            lowErrLimit = lex1GainLowErrLimit
            highWarnLimit = lex1GainHighWarnLimit
            lowWarnLimit = lex1GainLowWarnLimit
            
        elif erng == calConstant.CRNG_HEX8:
        
            highErrLimit = hex8GainHighErrLimit
            lowErrLimit = hex8GainLowErrLimit
            highWarnLimit = hex8GainHighWarnLimit
            lowWarnLimit = hex8GainLowWarnLimit
           
        else:
        
            highErrLimit = hex1GainHighErrLimit
            lowErrLimit = hex1GainLowErrLimit
            highWarnLimit = hex1GainHighWarnLimit
            lowWarnLimit = hex1GainLowWarnLimit 
            
        for tem in towers:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):
                    
                        gain = slopeData[tem,row,end,fe,erng,0]
                        
                        if gain > highWarnLimit or gain < lowWarnLimit:
                        
                            if gain > highErrLimit:
                                msg = 'gain %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (gain, highErrLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.error(msg)
                                status = 1
                            elif gain < lowErrLimit:
                                msg = 'gain %0.3f < %0.3f for %d,%s%s,%d,%s' % \
                                    (gain, lowErrLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.error(msg)
                                status = 1
                            
                            elif gain > highWarnLimit:
                                msg = 'gain %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (gain, highWarnLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.warning(msg)
                            else:
                                msg = 'gain %0.3f < %0.3f for %d,%s%s,%d,%s' % \
                                    (gain, lowWarnLimit, tem, calConstant.CROW[row],
                                     calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                log.warning(msg)                  
                                                       
    return status




def average(data, tems):

    av = 0
    for t in tems:
        av += Numeric.average(data[t,...], axis = None)
    return (av / len(tems))
    
    
def stddev(data, tems):

    av = 0
    for t in tems:
        sd = MLab.std(Numeric.ravel(data))
        av += (sd * sd)
        
    return math.sqrt(av / len(tems))
    
        

if __name__ == '__main__':

    usage = "muSlopeVal [-V] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
        
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('muSlopeVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-E:-W:-L:-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-R':
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
        elif o[0] == '-V':
            log.setLevel(logging.DEBUG)
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]
    
    # open and read XML Ped file

    log.info('Reading file %s', xmlName) 
    xmlFile = calCalibXML.calMuSlopeCalibXML(xmlName)
    slopeData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data
    
    valStatus = calcError(slopeData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms
        
        rootHists(slopeData, xmlName)

        # clean up

        rootFile.Close()
        
    # do simple stats 
    
    av = average(slopeData[...,calConstant.CRNG_LEX8,0], towers)
    sd = stddev(slopeData[...,calConstant.CRNG_LEX8,0], towers)
    log.info("LEX8 gain values average=%f, stddev=%f", av, sd)
    
    av = average(slopeData[...,calConstant.CRNG_LEX1,0], towers)
    sd = stddev(slopeData[...,calConstant.CRNG_LEX1,0], towers)
    log.info("LEX1 gain values average=%f, stddev=%f", av, sd)
    
    av = average(slopeData[...,calConstant.CRNG_HEX8,0], towers)
    sd = stddev(slopeData[...,calConstant.CRNG_HEX8,0], towers)
    log.info("HEX8 gain values average=%f, stddev=%f", av, sd)
    
    av = average(slopeData[...,calConstant.CRNG_HEX1,0], towers)
    sd = stddev(slopeData[...,calConstant.CRNG_HEX1,0], towers)
    log.info("HEX1 gain values average=%f, stddev=%f", av, sd)    

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
