"""
Validate CAL adc2nrg calibration data in XML format.  The command
line is:

adc2nrgVal [-V] [-r] [-R <root_file>] [-L <log_file>] <xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL adc2nrg calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL adc2nrg calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/08/03 03:26:43 $"
__version__   = "$Revision: 1.5 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calDacXML
import calConstant




# validation limits


leWarnLimit = 0.04
leErrLimit  = 0.06
heWarnLimit = 0.20
heErrLimit  = 0.30




def rootHists(energyData, fileName):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hists = [None, None]
    leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for eng in range(2):

        hName = "h_Summary_%s" % calConstant.CLEHE[eng]      
        hs = ROOT.TH1F(hName, 'adc2nrg: %s' % fileName, 100, 0.0, heErrLimit)
        hs.SetLineColor(eng + 1)
        hs.SetStats(False)
        
        for tem in towers:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):

                        e = energyData[tem, row, end, fe, eng]
                        hs.Fill(e)
                        
        hists[eng] = hs
        leg.AddEntry(hs, calConstant.CLEHE[eng], 'L')
        cs.Update()

    hMax = 0
    for eng in range(2):
        hs = hists[eng]
        if hs.GetMaximum() > hMax:
            hMax = hs.GetMaximum()

    for eng in range(2):
        if eng == 0:
            dopt = ''
            hs = hists[0]
            axis = hs.GetXaxis()
            axis.SetTitle('Gain (MeV/ADC)')
            axis.CenterTitle()
            axis = hs.GetYaxis()
            axis.SetTitle('Counts')
            axis.CenterTitle()
        else:
            dopt = 'SAME'
        hs = hists[eng]
        hs.SetMaximum(hMax)
        hs.Draw(dopt)
        cs.Update()
    
    leg.Draw()
    cs.Update()

    cs.Write()

    

def calcError(energyData):

    status = 0

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for eng in range(2):

                        if eng == 0:
                            eLimit = leErrLimit
                            wLimit = leWarnLimit
                        else:
                            eLimit = heErrLimit
                            wLimit = heWarnLimit
                            
                        e = energyData[tem, row, end, fe, eng]

                        if e <= 0.0:
                            log.error('%0.3f <= 0.0 for for %d,%s%s,%d,%s', e, tem, calConstant.CROW[row],
                                      calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                            status = 1

                        elif e > wLimit:
                            
                            if e > eLimit:
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (e, eLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                                log.error(msg)
                                status = 1
                            else:
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (e, wLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                                log.warning(msg)                        
                          

    return status



if __name__ == '__main__':

    usage = "adc2nrgVal [-V] [-r] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('adc2nrgVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-R':
            rootName = o[1]
            rootOutput = True
        elif o[0] == '-L':
            logName = o[1]
        elif o[0] == '-V':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-r':
            rootName = None
            rootOutput = True
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]
    ext = os.path.splitext(xmlName)
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

    log.debug('using LE warn limit %0.3f', leWarnLimit)
    log.debug('using LE err limit %0.3f', leErrLimit)
    log.debug('using HE warn limit %0.3f', heWarnLimit)
    log.debug('using HE err limit %0.3f', heErrLimit)

    # open and read XML adc2nrg file

    log.info("Reading file %s", xmlName)
    xmlFile = calDacXML.calEnergyXML(xmlName, 'adc2nrg')
    energyData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(energyData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(energyData, xmlName)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)

    
