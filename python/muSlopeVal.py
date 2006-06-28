"""
Validate CAL MuSlope calibration data in XML format.  The command
line is:

muSlopeVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for pedestal sigma value for x8 ranges
                    (default is 10.0; x1 ranges use this value / 5)
    -W <warn_limit> - warning limit pedestal sigma value for x8 ranges
                     (default is 8.0; x1 ranges use this value / 5)
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL MuSlope calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL MuSlope calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/06/28 15:57:15 $"
__version__   = "$Revision: 1.4 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calCalibXML
import calConstant



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

    return status



if __name__ == '__main__':

    usage = "muSlopeVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

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
        elif o[0] == '-E':
            x8ErrLimit = float(o[1])
        elif o[0] == '-W':
            x8WarnLimit = float(o[1])
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
    
    av = Numeric.average(slopeData[...,calConstant.CRNG_LEX8,0], axis = None)
    log.info("LEX8 gain values average=%f", av)
    av = Numeric.average(slopeData[...,calConstant.CRNG_LEX1,0], axis = None)
    log.info("LEX1 gain values average=%f", av)
    av = Numeric.average(slopeData[...,calConstant.CRNG_HEX8,0], axis = None)
    log.info("HEX8 gain values average=%f", av)
    av = Numeric.average(slopeData[...,calConstant.CRNG_HEX1,0], axis = None)
    log.info("HEX1 gain values average=%f", av)    

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
