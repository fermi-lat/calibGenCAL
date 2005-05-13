"""
Generate ULD Discriminator settings selected by Energy.  The command line is:

genULDsettings [-V] [-M <margin>] <uld2adc_file> <out_xml_file>

where:
    -V              = verbose; turn on debug output
    -M margin       = The saturation ADC margin value; default is '50'.
    <uld2adc_file>  = The ULD/ADC characterization table XML file.
    <out_xml_file>  = The ULD settings XML file to output.
"""



__facility__    = "Offline"
__abstract__    = "Generate ULD Discriminator settings selected by Energy"
__author__      = "D.L.Wood"
__date__        = "$Date: 2005/05/13 14:10:27 $"
__version__     = "$Revision: 1.1 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"


import sys, os, time
import logging
import getopt
import ConfigParser

import Numeric

import calFitsXML
import calDacXML



if __name__ == '__main__':

    usage = "genULDsettings [-V] [-M margin] <uld2adc_file> <out_xml_file>"

    margin = 50    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger()
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
        elif o[0] == '-M':
            margin = int(o[1])
        
    args = opts[1]
    if len(args) != 2:
        log.error(usage)
        sys.exit(1)

    uldName = args[0]
    outName = args[1]

    # read ULD/ADC characterization table

    log.info("genULDsettings: reading ULD ADC file %s", uldName)
    fio = calFitsXML.calFitsXML(fileName = uldName, mode = calFitsXML.MODE_READONLY)
    adcData = fio.read()
    fio.close()

    # find saturation values    

    sat = [127, 127, 127]
    dacData = Numeric.zeros((16, 8, 2, 12), Numeric.Int16)

    for row in range(8):
        for end in range(2):
            for fe in range(12):
                for erng in range(3):
                    adcmax = adcData[erng, 0, row, end, fe, 127]
                    for dac in range(126, 64, -1):
                        adc = adcData[erng, 0, row, end, fe, dac] + margin
                        if adc < adcmax:
                            break
                    sat[erng] = (dac - 1)

                dacData[0, row, end, fe] = min(sat)                        

    print dacData[0,...]
                        
    # write output file                    

    log.info('genULDsettings: writing ULD settings file %s', outName)
    fio = calDacXML.calDacXML(outName, 'rng_uld_dac', calDacXML.MODE_CREATE)
    fio.write(dacData)
    fio.close()
    
    sys.exit(0)
    