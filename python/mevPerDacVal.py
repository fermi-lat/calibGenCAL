"""
Validate CAL MevPerDac calibration data in XML format.  The command
line is:

mevPerDacVal [-V] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] [-L <log_file>] <xml_file>

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
__abstract__  = "Validate CAL MevPerDac calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/25 21:32:48 $"
__version__   = "$Revision: 1.4 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import getopt
import logging

import Numeric

import calCalibXML
import calConstant


BIGSMALL = ('BIG', 'SMALL')


def rootHists(errData):

    # create summary big histogram

    cs = ROOT.TCanvas('c_Summary_Big', 'Summary_Big', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary_Big"      
    hs = ROOT.TH1F(hName, 'MevPerDac_Big', 100, 0.0, bigErrLim)

    for tem in towers:
        for row in range(8):
            for fe in range(12):
                    err = errData[tem, row, fe, 0]
                    hs.Fill(err)
                    cs.Update()                       
    
    hs.Draw()
    cs.Update()

    cs.Write()

    # create summary small histograms

    cs = ROOT.TCanvas('c_Summary_Small', 'Summary_Small', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary_Small"      
    hs = ROOT.TH1F(hName, 'MevPerDac_Small', 100, 0.0, smallErrLim)

    for tem in towers:
        for row in range(8):
            for fe in range(12):
                    err = errData[tem, row, fe, 1]
                    hs.Fill(err)
                    cs.Update()                       
    
    hs.Draw()
    cs.Update()

    cs.Write()    



def calcError(energyData):

    errs = Numeric.zeros((16, 8, 12, 2), Numeric.PyObject)
    status = 0

    for tem in towers:
        for row in range(8):
            for fe in range(12):
                for val in range(2):

                    if val == 0:
                        errLim = bigErrLim
                        warnLim= bigWarnLim
                    else:
                        errLim = smallErrLim
                        warnLim = smallWarnLim

                    ex = energyData[tem, row, fe, val]
                    
                    if ex > warnLim:
                            
                        if ex > errLim:
                            msg = 'mevPerDacVal: %0.3f > %0.3f for %d,%s,%d,%s' % \
                                (ex, errLim, tem, calConstant.CROW[row], fe, BIGSMALL[val])
                            log.error(msg)
                            status = 1
                        else:
                            msg = 'mevPerDacVal: %0.3f > %0.3f for %d,%s,%d,%s' % \
                                (ex, warnLim, tem, calConstant.CROW[row], fe, BIGSMALL[val])
                            log.warning(msg)

                    errs[tem, row, fe, val] = ex

    return (status, errs)



if __name__ == '__main__':

    usage = "usage: mevPerDacVal [-V] [-L <log_file>] [-E <err_limit>] [-W <warn_limit>] [-R <root_file>] <xml_file>"

    rootOutput = False
    smallErrLim = 30.0
    smallWarnLim = 20.0

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
            smallErrLim = float(o[1])
        elif o[0] == '-W':
            smallWarnLim = float(o[1])
        elif o[0] == '-L':
            if os.path.exists(o[1]):
                log.warning('mevPerDacVal: deleting old log file %s', o[1])
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
    bigErrLim = (smallErrLim / 5)
    bigWarnLim = (smallWarnLim / 5)

    log.debug('mevPerDacVal: using input file %s', xmlName)
    log.debug('mevPerDacVal: using err limit %0.3f for Big diodes', bigErrLim)
    log.debug('mevPerDacVal: using err limit %0.3f for Small diodes', smallErrLim)
    log.debug('mevPerDacVal: using warn limit %0.3f for Big diodes', bigWarnLim)
    log.debug('mevPerDacVal: using warn limit %0.3f Small diodes', smallWarnLim)

    # open and read XML Ped file

    xmlFile = calCalibXML.calMevPerDacCalibXML(xmlName)
    energyData = xmlFile.read()
    towers = xmlFile.getTowers()
    xmlFile.close()

    # validate calibration data

    valStatus = 0
    (valStatus, errData) = calcError(energyData)

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

    log.info('mevPerDacVal: validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    