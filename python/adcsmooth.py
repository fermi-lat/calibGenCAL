"""
Tool to smooth CAL XML file ADC/DAC characterization data.    The command
line is:

adcsmooth [-V] <in_file> <out_file>

where:
    [-V]        = verbose; turn on debug output
    <in_file>   = raw ADC/DAC XML characterization file name
    <out_file>  = smoothed output XML characterization file name
"""


__facility__  = "Offline"
__abstract__  = "Tool to smooth CAL XML file ADC/DAC characterization data"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/09/15 12:50:30 $"
__version__   = "$Revision: 1.6 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import os, sys
import logging
import getopt

import Numeric

import calADCFilter
import calFitsXML
import calConstant



#######################################################################################


if __name__ == '__main__':

    usage = "usage: adcsmooth [-V] <in_file> <out_file>"

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('adcsmooth')
    log.setLevel(logging.INFO)
    debug = False

    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)
            debug = True
        
    args = opts[1]    

    if len(args) != 2:
        log.error(usage)
        sys.exit(1)
    inName = args[0]    
    outName = args[1]

    # open and read input XML file

    log.info("reading file %s", inName)    

    inFile = calFitsXML.calFitsXML(fileName = inName)
    inData = inFile.read()
    tlist = inFile.getTowers()
    info = inFile.info()
    inFile.close()

    type = info['TTYPE1']
    if type == 'fhe_dac':
        fileType = calADCFilter.DAC_TYPE_FHE
    elif type == 'fle_dac':
        fileType = calADCFilter.DAC_TYPE_FLE
    elif type == 'log_acpt':
        fileType = calADCFilter.DAC_TYPE_LAC
    elif type == 'rng_uld_dac':
        fileType = calADCFilter.DAC_TYPE_ULD
    else:
        log.error('file type %s not supported', str(type))
        sys.exit(1)

    # run smoothing algorithm

    log.info("smoothing data for towers: %s", str(tlist))

    filter = calADCFilter.calADCFilter(type = fileType, smoothing = True,
                                       verbose = debug)
    outData = filter.run(inData, tlist)

    # create output FITS file

    log.info("writing file %s", outName)

    sn = {}
    for n in calConstant.CFACE:
        name = 'CALSN%s' % n
        s = info[name]
        if s is not None:
            sn[n] = s 

    la = []
    for n in range(1,8):
        name = 'LAXIS%d' % n
        l = info[name]
        if l is not None:
            la.append(l)
    la.append(info['TTYPE1'])    

    outFile = calFitsXML.calFitsXML(fileName = outName, mode = calFitsXML.MODE_CREATE, labels = la, \
        calSNs = sn, dataset = info['DATASET'], lrefgain = info['LREFGAIN'], hrefgain = info['HREFGAIN'], \
        pedFile = info['PEDFILE'], erng = info['ERNG'], reportName = info['RPTNAME'], runId = info['RUNID'], \
        comment = info['COMMENT'], type = info['TTYPE1'])

    outFile.write(outData, tems = tlist)
    outFile.close()

    sys.exit(0)
