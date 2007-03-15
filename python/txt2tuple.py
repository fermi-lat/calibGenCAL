"""
Tool to create ROOT ntuple file from csv file. First line of text is used as column headers
txt2tuple input.txt output.root
some assumptions are made about calibGenCAL delimited TXT file conventions
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate ROOT ntuple file from txt."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2007/03/06 17:40:37 $"
__version__   = "$Revision: 1.6 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

import sys, os
import logging
import array
import re
import csv

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

    ### open input 'csv' file ###
    csv_sample = file(inPath).read(8192)
    dialect = csv.Sniffer().sniff(csv_sample)
    has_header = csv.Sniffer().has_header(csv_sample)
    infile = csv.reader(open(inPath,"r"),dialect = dialect)

    # skip optional header in first line
    header_list = infile.next()
    header_cols = ':'.join(header_list)
    nTXTFields = len(header_list)

    # create ROOT ntuple definition string for column
    # names

    # create ROOT File & Ntuple
    outfile = ROOT.TFile(outPath, "RECREATE")
    tuple_name = os.path.splitext(os.path.basename(outPath))[0]
    # strip bad characters
    tuple_name = re.compile('[. ]+').sub('_',tuple_name)
    # take first alpha string
    tuple_name = tuple_name.split('_')[0]

    tuple = ROOT.TNtuple(tuple_name,
                         tuple_name,
                         header_cols)

    for line in infile:
        vals = [float(x) for x in line]
        data = array.array('f',vals)
        tuple.Fill(data)


    log.info('Writing output file %s', outPath)

    outfile.Write()
    outfile.Close()

    sys.exit(0)                            
