"""
Validate CAL threshold bias offset calibration data in XML format.  The command
line is:

biasVal [-V] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL bias offset calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL bias offset calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/09/20 21:11:13 $"
__version__   = "$Revision: 1.1 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calDacXML
import calConstant



# validation limits


warnLimit   = 120.0
errLimit    = 180.0




def rootHists(biasData, fileName):

    # create summary histogram

    cs = ROOT.TCanvas('c_Summary', 'Summary', -1)
    cs.SetGrid()
    cs.SetLogy()

    hists = [None, None]
    leg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for eng in range(2):

        hName = "h_Summary_%s" % calConstant.CLEHE[eng]      
        hs = ROOT.TH1F(hName, 'bias: %s' % fileName, 100, 0.0, errLimit)
        hs.SetLineColor(eng + 1)
        hs.SetStats(False)
        
        for tem in towers:
            for row in range(calConstant.NUM_ROW):
                for end in range(calConstant.NUM_END):
                    for fe in range(calConstant.NUM_FE):

                        e = biasData[tem, row, end, fe, eng]
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
            axis.SetTitle('Bias (ADC)')
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



def calcError(biasData):

    status = 0

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for end in range(calConstant.NUM_END):
                for fe in range(calConstant.NUM_FE):
                    for eng in range(2):

                        e = biasData[tem, row, end, fe, eng]

                        if e <= 0.0:
                            log.error('%0.3f <= 0.0 for for %d,%s%s,%d,%s', e, tem, calConstant.CROW[row],
                                      calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                            status = 1

                        elif e > warnLimit:
                            
                            if e > errLimit:
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (e, errLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                                log.error(msg)
                                status = 1
                            else:
                                msg = '%0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                    (e, warnLimit, tem, calConstant.CROW[row],
                                    calConstant.CPM[end], fe, calConstant.CLEHE[eng])
                                log.warning(msg)                        

    return status



if __name__ == '__main__':

    usage = " biasVal [-V] [-L <log_file>] [-R <root_file>] <xml_file>"

    rootOutput = False
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('biasVal')
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
            errLimit = float(o[1])
        elif o[0] == '-W':
            warnLimit = float(o[1])
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

    log.debug('using warn limit %0.3f', warnLimit)
    log.debug('using err limit %0.3f', errLimit)

    # open and read XML adc2nrg file

    log.info("Reading file %s", xmlName)
    xmlFile = calDacXML.calEnergyXML(xmlName, 'thrBias')
    biasData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = calcError(biasData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(biasData, xmlName)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    