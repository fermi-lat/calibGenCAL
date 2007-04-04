"""
Convert cidac2adc (intNonlin) xml data file into root tuples (1 per channel)

inl2tuple input.xml output.root
where:
     input.xml intNonlin XML file
     output.root output ROOT file

"""

__facility__  = "Offline"
__abstract__  = "Convert cidac2adc (intNonlin) xml data file into root tuples (1 per channel)"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/02/02 20:28:42 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import getopt
import Numeric
import ROOT

import calCalibXML
import calConstant
import zachUtil

#######################################################################################
if __name__ == '__main__':

    # constants
    usage      = "inl2tuple input.xml output.root"

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('inl2tuple')
    log.setLevel(logging.INFO)

    # check command line
    try:
        # currently no args supported
        (optList,args) = getopt.getopt(sys.argv[1:],"")
    except getopt.GetoptError:
        log.error(usage)
        sys.exit(1)

    if len(args) != 2:
        log.error("Wrong # of paramters: %s"%usage)
        sys.exit(1)

    (inPath, outPath) = args

    # read in data files.
    log.info("Reading input %s"%inPath)
    inFile = calCalibXML.calIntNonlinCalibXML(inPath)
    inData = inFile.read()
    (inLen, inDAC, inADC) = inData
    inTwrSet = inFile.getTowers()

    # open output ROOT output file
    outFile = ROOT.TFile(outPath,"RECREATE")

    # create tuples per channel
    # print out txt file.
    for twr in inTwrSet:
        for lyr in range(calConstant.NUM_LAYER):
            # calCalibXML uses 'row' indexing, not layer
            row = calCalibXML.layerToRow(lyr)
            for col in range(calConstant.NUM_FE):
                for face in range(calConstant.NUM_END):
                    online_face = calConstant.offline_face_to_online[face]
                    for rng in range(calConstant.NUM_RNG):
                        ## retrieve data for this channel ##
                        len = inLen[rng][twr, row, online_face, col]
                        
                        # skip channel if data is empty
                        if (len <= 1):
                            continue
                        
                        dac = inDAC[rng][twr, row, online_face, col, 0:len]
                        adc = inADC[rng][twr, row, online_face, col, 0:len]

                        # build tuple object
                        channel_str = "cidac2adc_%s_%s_%s_%s_%s"%(twr, lyr, col, face, rng)
                        tpl = ROOT.TNtuple(channel_str,
                                           channel_str,
                                           "dac:adc")

                        for idx in range(len):
                            tpl.Fill(dac[idx], adc[idx])

                        tpl.Write()
    
    # close & write ROOT output file
    outFile.Write()
    outFile.Close()
    
    sys.exit(0)
