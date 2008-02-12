"""
Validate CAL MevPerDac calibration data in XML format.  The command
line is:

mevPerDacVal [-V] [-r] [-R <root_file>] [-L <log_file>] <xml_file>

where:
    -r             - generate ROOT output with default name
    -R <root_file> - output validation diagnostics in ROOT file
    -L <log_file>  - save console output to log text file
    -V             - verbose; turn on debug output
    <xml_file> The CAL MevPerDac calibration XML file to validate.    
"""


__facility__  = "Offline"
__abstract__  = "Validate CAL MevPerDac calibration data in XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2008/02/11 21:35:58 $"
__version__   = "$Revision: 1.10 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import sys, os
import getopt
import logging

import numarray

import calCalibXML
import calConstant




# validation limits


smallErrLim     = 5.0
smallWarnLim    = 2.5
bigErrLim       = 1.0
bigWarnLim      = 0.5



BIGSMALL = ('BIG', 'SMALL')




def rootHists(errData, fileName):

    # create summary big histogram

    cs = ROOT.TCanvas('c_Summary_Big', 'Summary_Big', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary_Big"      
    hs = ROOT.TH1F(hName, 'MevPerDac_Big %s' % fileName, 100, 0.0, bigErrLim)

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for fe in range(calConstant.NUM_FE):
                    err = errData[tem, row, fe, 0]
                    hs.Fill(err)
               
    
    axis = hs.GetXaxis()
    axis.SetTitle('Gain (MeV/DAC)')
    axis.CenterTitle()
    axis = hs.GetYaxis()
    axis.SetTitle('Counts')
    axis.CenterTitle()
    
    hs.Draw()
    cs.Update()

    cs.Write()

    # create summary small histograms

    cs = ROOT.TCanvas('c_Summary_Small', 'Summary_Small', -1)
    cs.cd()
    cs.SetGrid()
    cs.SetLogy()

    hName = "h_Summary_Small"      
    hs = ROOT.TH1F(hName, 'MevPerDac_Small: %s' % fileName, 100, 0.0, smallErrLim)

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for fe in range(calConstant.NUM_FE):
                    err = errData[tem, row, fe, 1]
                    hs.Fill(err)
     
    axis = hs.GetXaxis()
    axis.SetTitle('Gain (MeV/DAC)')
    axis.CenterTitle()
    axis = hs.GetYaxis()
    axis.SetTitle('Counts')
    axis.CenterTitle()         
    
    hs.Draw()
    cs.Update()

    cs.Write()    



def calcError(energyData):
    import numarray.objects
    errs = numarray.objects.array(None,(calConstant.NUM_TEM,
                                        calConstant.NUM_ROW,
                                        calConstant.NUM_FE, 2))
    status = 0

    for tem in towers:
        for row in range(calConstant.NUM_ROW):
            for fe in range(calConstant.NUM_FE):
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
                            msg = '%0.3f > %0.3f for %d,%s,%d,%s' % \
                                (ex, errLim, tem, calConstant.CROW[row], fe, BIGSMALL[val])
                            log.error(msg)
                            status = 1
                        else:
                            msg = '%0.3f > %0.3f for %d,%s,%d,%s' % \
                                (ex, warnLim, tem, calConstant.CROW[row], fe, BIGSMALL[val])
                            log.warning(msg)

                    errs[tem, row, fe, val] = ex

    return (status, errs)



if __name__ == '__main__':


    rootOutput = False
    logName = None
    
    # setup logger

    logging.basicConfig()
    log = logging.getLogger('mevPerDacVal')
    log.setLevel(logging.INFO)

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-R:-L:-V-r")
    except getopt.GetoptError:
        log.error(__doc__)
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
        log.error(__doc__)
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
    
    log.debug('Using err limit %0.3f for Big diodes', bigErrLim)
    log.debug('Using err limit %0.3f for Small diodes', smallErrLim)
    log.debug('Using warn limit %0.3f for Big diodes', bigWarnLim)
    log.debug('Using warn limit %0.3f Small diodes', smallWarnLim)

    # open and read XML Ped file

    log.info("Reading file %s", xmlName)
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

        log.info("Writing file %s", rootName)
        ROOT.gROOT.Reset()
        rootFile = ROOT.TFile(rootName, "recreate")

        # write error histograms

        rootHists(errData, xmlName)        

        # clean up

        rootFile.Close()

    # report results

    if valStatus == 0:
        statusStr = 'PASSED'
    else:
        statusStr = 'FAILED'

    log.info('Validation %s for file %s', statusStr, xmlName)
    sys.exit(valStatus)
    
