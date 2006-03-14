"""
Validate CAL adc2nrg calibration data in XML format.  The command
line is:

adc2nrgVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for pedestal sigma value for x8 ranges
                    (default is 10.0; x1 ranges use this value / 5)
    -W <warn_limit> - warning limit pedestal sigma value for x8 ranges
                     (default is 8.0; x1 ranges use this value / 5)
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL adc2nrg calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL adc2nrg calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/02/16 20:15:32 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calDacXML
import calConstant




def rootHists(energyData):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hists = [None, None]
    leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for eng in range(2):

        hName = "h_Summary_%s" % calConstant.CLEHE[eng]      
        hs = ROOT.TH1F(hName, 'adc2nrg', 100, 0.0, heErrLimit)
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

    errs = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_FE, 2), Numeric.PyObject)
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

    usage = "adc2nrgVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    leWarnLimit = 0.04
    leErrLimit = 0.06

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('adc2nrgVal')
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
            leErrLimit = float(o[1])
        elif o[0] == '-W':
            leWarnLimit = float(o[1])
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

    heWarnLimit = (leWarnLimit * 5.0)
    heErrLimit = (leErrLimit * 5.0)

    log.debug('Using input file %s', xmlName)
    log.debug('using LE warn limit %0.3f', leWarnLimit)
    log.debug('using LE err limit %0.3f', leErrLimit)
    log.debug('using HE warn limit %0.3f', heWarnLimit)
    log.debug('using HE err limit %0.3f', heErrLimit)

    # open and read XML adc2nrg file

    xmlFile = calDacXML.calEnergyXML(xmlName, 'adc2nrg')
    energyData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(energyData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(energyData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)

    
