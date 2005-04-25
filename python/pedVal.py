"""
Validate CAL Ped calibration data in XML format.  The command
line is:

pedVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file>

where:

    -R <root_file> - output validation diagnostics in ROOT file
    -E <err_limit> - error limit for pedestal sigma value for x8 ranges
                    (default is 10.0; x1 ranges use this value / 5)
    -W <warn_limit> - warning limit pedestal sigma value for x8 ranges
                     (default is 8.0; x1 ranges use this value / 5)
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL Int_Nonlin calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL Ped calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/25 19:20:59 $"
__version__   = "$Revision: 1.3 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
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
                                eLim = x8ErrLimit
                                wLim = x8WarnLimit
                            else:
                                eLim = x1ErrLimit
                                wLim = x1WarnLimit
                    
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

    usage = "usage: pedVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    x8ErrLimit = 10.0
    x8WarnLimit = 6.0

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
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
                log.warning('pedVal: deleting old log file %s', o[1])
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
    x1ErrLimit = (x8ErrLimit / 5.0)
    x1WarnLimit = (x8WarnLimit / 5.0)

    log.debug('pedVal: using input file %s', xmlName)
    log.debug('pedVal: using sig err limit %0.3f for x8 ranges', x8ErrLimit)
    log.debug('pedVal: using sig err limit %0.3f for x1 ranges', x1ErrLimit)
    log.debug('pedVal: using sig warn limit %0.3f for x8 ranges', x8WarnLimit)
    log.debug('pedVal: using sig warn limit %0.3f for x1 ranges', x1WarnLimit)

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

    log.info('pedVal: validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    