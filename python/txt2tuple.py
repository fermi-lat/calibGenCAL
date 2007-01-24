"""
Tool to create ROOT ntuple file from csv file. First line of text is used as column headers
txt2tuple input.txt output.root
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate ROOT ntuple file from txt."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/01/12 15:59:16 $"
__version__   = "$Revision: 1.3 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import array
import re

import ROOT


#######################################################################################

if __name__ == '__main__':

    # constants
    usage      = "txt2tuple input.txt output.root"

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('txt2tuple')
    log.setLevel(logging.INFO)

    # parse commandline
    if (len(sys.argv) != 3):
        log.error("Need 2 filenames: " + usage)
        sys.exit(1)
        
    inPath = sys.argv[1]
    outPath = sys.argv[2]

    # read input file
    inFile = open(inPath, 'r')

    lines = inFile.readlines()

    # read in first line for column headers
    firstline = lines.pop(0)
    col_names = firstline.split()
    nTXTFields = len(col_names)

    # create ROOT ntuple definition string for column
    # names
    col_def = ':'.join(col_names)

    # create ROOT File & Ntuple
    outfile = ROOT.TFile(outPath, "RECREATE")
    tuple_name = os.path.splitext(os.path.basename(outPath))[0]
    # strip bad characters
    tuple_name = re.compile('[.-]').sub('_',tuple_name)
    tuple = ROOT.TNtuple(tuple_name,
                         tuple_name,
                         col_def)

    for line in lines:
        vals = line.split()
        vals = [float(x) for x in vals]
        data = array.array('f',vals)
        tuple.Fill(data)


    log.info('Writing output file %s', outPath)

    outfile.Write()
    outfile.Close()

    sys.exit(0)                            

    
