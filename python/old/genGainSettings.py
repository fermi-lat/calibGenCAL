"""
Generate snapshot fragment of CAL gain settings.  This will generate GCFE Config 0 Register settings values.
The command line is:

genGainSettings [-V] <leGain> <heGain> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    <leGain>        = the LE gain value
    <heGain>        = the HE gain value
    <out_xml_file>  = The ULD settings XML file to output.
"""



__facility__    = "Offline"
__abstract__    = "Generate CAL gain settings for GCFE Config 0 Register"
__author__      = "D.L.Wood"
__date__        = "$Date: 2007/03/20 19:23:47 $"
__version__     = "$Revision: 1.2 $, $Author: fewtrell $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import logging
import getopt

import numarray

import calDacXML



if __name__ == '__main__':

    usage = "genGainSettings [-V] <leGain> <heGain> <out_xml_file>"


    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genGainSettings')
    log.setLevel(logging.INFO)

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
        
    args = opts[1]
    if len(args) != 3:
        log.error(usage)
        sys.exit(1)

    leGain = int(args[0])
    heGain = int(args[1])
    outName = args[2]

    if leGain > 7:
        log.error('LE gain value %d out of range', leGain)
        sys.exit(1)
    if heGain > 15:
        log.error('HE gain value %d out of range', heGain)
        sys.exit(1)

    log.debug('Using LE gain %d', leGain)
    log.debug('Using HE gain %d', heGain)

    # create config 0 register values

    dacData = numarray.zeros((16, 8, 2, 12), numarray.Int16)
    reg = (leGain | (heGain << 3))
    log.debug('Using Config 0 register value %hx', reg)
    dacData = (dacData + reg);

    # write output file                    

    log.info('Writing gain settings file %s', outName)
    fio = calDacXML.calDacXML(outName, 'config_0', calDacXML.MODE_CREATE)
    tlist = range(16)
    fio.write(dacData, filename = outName, leGain = leGain, heGain = heGain,
              method = 'genGainSettings:%s' % __release__, tems = tlist)
    fio.close()    


    sys.exit(0)

    
