"""
Validate CAL Ped calibration data in XML format.  The command
line is:

pedVal [-V] [-R <root_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -V             - verbose; turn on debug output
    <xml_file> The CAL Int_Nonlin calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Ped calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/20 16:37:02 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys
import getopt
import logging

import Numeric

import calCalibXML
import calConstant


def rootHists(errData):

    # create summary x8 sig histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_x8', 'Summary_x8', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(0, 4, 2):

        hName = "h_Summary_x8_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'Ped_x8_Summary', 100, 0.0, errLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):             
                    for erng in range(0, 4, 2):
                        hs = sumHists[erng]
                        eStr = errData[tem, row, end, fe, erng]
                        err = eval(eStr)
                        for e in err:
                            hs.Fill(e)
                        cs.Update()                       
    
    for erng in range(0, 4, 2):

        hs = sumHists[erng]
        if erng == 0:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()

    # create summary x1 sig histograms

    sumHists = [None, None, None, None]
    cs = ROOT.TCanvas('c_Summary_x1', 'Summary_x1', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()
    sumLeg = ROOT.TLegend(0.88, 0.88, 0.99, 0.99)

    for erng in range(1, 4, 2):

        hName = "h_Summary_x1_%s" % calConstant.CRNG[erng]       
        hs = ROOT.TH1F(hName, 'Ped_x1_Summary', 100, 0.0, errLimit)
        hs.SetLineColor(erng + 1)
        hs.SetStats(False)
        sumHists[erng] = hs
        sumLeg.AddEntry(hs, calConstant.CRNG[erng], 'L')
        cs.Update()

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):             
                    for erng in range(1, 4, 2):
                        hs = sumHists[erng]
                        eStr = errData[tem, row, end, fe, erng]
                        err = eval(eStr)
                        for e in err:
                            hs.Fill(e)
                        cs.Update()                       
    
    for erng in range(1, 4, 2):

        hs = sumHists[erng]
        if erng == 1:
            dopt = ''
        else:
            dopt = 'SAME'
        hs.Draw(dopt)
        cs.Update()

    sumLeg.Draw()
    cs.Update()
    cs.Write()



def calcError(pedData):

    errs = Numeric.zeros((16, 8, 2, 12, 4), Numeric.PyObject)
    status = 0

    for tem in towers:
        for row in range(8):
            for end in range(2):
                for fe in range(12):
                    for erng in range(4):

                        err = []
                            
                        for ex in pedData[tem, row, end, fe, erng, 1]:
                            
                            err.append(ex)
                            if erng == 0 or erng == 2:
                                eLim = errLimit
                                wLim = warnLimit
                            else:
                                eLim = (errLimit / 5)
                                wLim = (warnLimit / 5)
                    
                            if ex > wLim:
                            
                                if ex > eLim:
                                    msg = 'pedVal: %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                        (ex, eLim, tem, calConstant.CROW[row],
                                         calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.error(msg)
                                    status = 1
                                else:
                                    msg = 'pedVal: %0.3f > %0.3f for %d,%s%s,%d,%s' % \
                                        (ex, wLim, tem, calConstant.CROW[row],
                                         calConstant.CPM[end], fe, calConstant.CRNG[erng])
                                    log.warning(msg)

                        errs[tem, row, end, fe, erng] = repr(err)

    return (status, errs)



if __name__ == '__main__':

    usage = "usage: pedVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    errLimit = 10.0
    warnLimit = 8.0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-E:-W:-V")
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
        elif o[0] == '-V':
            log.setLevel(logging.DEBUG)
        
    args = opts[1]
    if len(args) != 1:
        log.error(usage)
        sys.exit(1)    

    xmlName = args[0]

    log.debug('pedVal: using input file %s', xmlName)
    log.debug('pedVal: using sig err limit %0.3f for x8 ranges', errLimit)
    log.debug('pedVal: using sig err limit %0.3f for x1 ranges', (errLimit / 5))
    log.debug('pedVal: using sig warn limit %0.3f x8 ranges', warnLimit)
    log.debug('pedVal: using sig warn limit %0.3f x8 ranges', (warnLimit / 5))

    # open and read XML Ped file

    xmlFile = calCalibXML.calPedCalibXML(xmlName)
    pedData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()


    # validate calibration data

    (valStatus, errData) = calcError(pedData)

    # create ROOT output file
    
    if rootOutput:

        import ROOT

        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('asymVal: validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    