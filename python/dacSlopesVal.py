"""
Validate CAL DacSlopes calibration data in XML format.  The command
line is:

tholdCIVal [-V] [-L <log_file>] [-R <root_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>   - save console output to log text file
    -V              - verbose; turn on debug output
    <xml_file> The CAL DacSlopes calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL DacSlopes calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/07/03 19:28:23 $"
__version__   = "$Revision: 1.19 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os, math
import getopt
import logging
import array

import Numeric

import calCalibXML
import calConstant
                  
                  

# validation limits





def rootHists(dacData, uldData, rangeData, fileName):

    # create summary ULD slope histograms

    sumHists = [None, None, None]
    cs = ROOT.TCanvas('c_Slopes_ULD', 'Slopes_ULD', -1)
    cs.SetGrid()
    cs.SetLogx()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(3):

        hName = "h_Slopes_ULD_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'DacSlopes_Slope_ULD: %s' % fileName, 100, 0, 100)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Slope (MeV/DAC)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
     
    for tem in towers:          
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(3):
                        hs = sumHists[erng]
                        uld = uldData[erng,tem,row,end,fe,0]                        
                        hs.Fill(uld)

   
    hMax = 0
    for erng in range(3):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for erng in range(3):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()

   
    # create summary ULD saturation histograms

    sumHists = [None, None, None]
    cs = ROOT.TCanvas('c_Saturation_ULD', 'Saturation_ULD', -1)
    cs.SetGrid()
    cs.SetLogx()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(3):

        hName = "h_Saturation_ULD_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'DacSlopes_Saturation_ULD: %s' % fileName, 100, 0, 10000)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Saturation (MeV)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()
        
     
    for tem in towers:          
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for erng in range(3):
                        hs = sumHists[erng]
                        uld = uldData[erng,tem,row,end,fe,2]                        
                        hs.Fill(uld)

   
    hMax = 0
    for erng in range(3):
        hs = sumHists[erng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for erng in range(3):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()
    

    # create summary DAC slope histograms

    sumHists = [None, None, None]
    cs = ROOT.TCanvas('c_Slopes_DAC', 'Slopes_DAC', -1)
    cs.SetGrid()
    cs.SetLogx()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
    dacNames = ('LAC', 'FLE', 'FHE')

    for dac in range(3):

        hName = "h_Slopes_DAC_%s" % dacNames[dac]       
        hs = ROOT.TH1F(hName, 'DacSlopes_Slopes_DAC: %s' % fileName, 100, 0, 100)
        hs.SetLineColor(dac + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Slope (MeV/DAC)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[dac] = hs
        sumLeg.AddEntry(hs, dacNames[dac], 'L')
        cs.Update()
        
     
    for tem in towers:          
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for dac in range(3):
                        hs = sumHists[dac]
                        d = dacData[tem,row,end,fe, (dac * 2)]                        
                        hs.Fill(d)

   
    hMax = 0
    for dac in range(3):
        hs = sumHists[dac]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for dac in range(3):

        hs = sumHists[dac]
        if dac == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()
    
    
    # create summary DAC offset histograms

    sumHists = [None, None, None]
    cs = ROOT.TCanvas('c_Offsets_DAC', 'Offsets_DAC', -1)
    cs.SetGrid()
    cs.SetLogx()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)
    
    dacNames = ('LAC', 'FLE', 'FHE')

    for dac in range(3):

        hName = "h_Offsets_DAC_%s" % dacNames[dac]       
        hs = ROOT.TH1F(hName, 'DacSlopes_Offsets_DAC: %s' % fileName, 100, 0, 1000)
        hs.SetLineColor(dac + 1)
        hs.SetStats(False)
        axis = hs.GetXaxis()
        axis.SetTitle('Offset (MeV)')
        axis.CenterTitle()
        axis = hs.GetYaxis()
        axis.SetTitle('Counts')
        axis.CenterTitle()
        sumHists[dac] = hs
        sumLeg.AddEntry(hs, dacNames[dac], 'L')
        cs.Update()
        
     
    for tem in towers:          
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for dac in range(3):
                        hs = sumHists[dac]
                        d = dacData[tem,row,end,fe, (dac * 2) + 1]                        
                        hs.Fill(d)

   
    hMax = 0
    for dac in range(3):
        hs = sumHists[dac]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()    
        
    for dac in range(3):

        hs = sumHists[dac]
        if dac == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()



def calcError(dacData, uldData, rangeData):

    status = 0

    return status



if __name__ == '__main__':

    usage = "usage: dacSlopesVal [-V] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('dacSlopesVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V")
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

    # open and read XML DacSlopes file

    log.info('Reading file %s', xmlName) 
    xmlFile = calCalibXML.calDacSlopesCalibXML(xmlName)
    (dacData, uldData, rangeData) = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(dacData, uldData, rangeData)    

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info('Creating file %s' % rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(dacData, uldData, rangeData, xmlName)        

        # clean up

        rootFile.Close()
        
        
    # do simple stats
    
    av = Numeric.average(dacData[...,0], axis = None)
    log.info("LAC Mev/DAC slope average = %f", av)
    av = Numeric.average(dacData[...,1], axis = None)
    log.info("LAC Mev offset average = %f", av)
        
    av = Numeric.average(dacData[...,2], axis = None)
    log.info("FLE Mev/DAC slope average = %f", av)
    av = Numeric.average(dacData[...,3], axis = None)
    log.info("FLE Mev offset average = %f", av)
    
    av = Numeric.average(dacData[...,4], axis = None)
    log.info("FHE Mev/DAC slope average = %f", av)
    av = Numeric.average(dacData[...,5], axis = None)
    log.info("FHE Mev offset average = %f", av)
        
    av = Numeric.average(uldData[calConstant.CRNG_LEX8,...,0], axis = None)
    log.info("ULD LEX8 Mev/DAC slope average = %f", av) 
    av = Numeric.average(uldData[calConstant.CRNG_LEX8,...,1], axis = None) 
    log.info("ULD LEX8 MeV offset average = %f", av)
    av = Numeric.average(uldData[calConstant.CRNG_LEX8,...,2], axis = None)
    log.info("ULD LEX8 MeV saturation average = %f", av)
    
    av = Numeric.average(uldData[calConstant.CRNG_LEX1,...,0], axis = None)
    log.info("ULD LEX1 Mev/DAC slope average = %f", av) 
    av = Numeric.average(uldData[calConstant.CRNG_LEX1,...,1], axis = None) 
    log.info("ULD LEX1 MeV offset average = %f", av)
    av = Numeric.average(uldData[calConstant.CRNG_LEX1,...,2], axis = None)
    log.info("ULD LEX1 MeV saturation average = %f", av)
    
    av = Numeric.average(uldData[calConstant.CRNG_HEX8,...,0], axis = None)
    log.info("ULD HEX8 Mev/DAC slope average = %f", av) 
    av = Numeric.average(uldData[calConstant.CRNG_HEX8,...,1], axis = None) 
    log.info("ULD HEX8 MeV offset average = %f", av)
    av = Numeric.average(uldData[calConstant.CRNG_HEX8,...,2], axis = None)
    log.info("ULD HEX8 MeV saturation average = %f", av)

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)        
        
    sys.exit(valStatus)

    