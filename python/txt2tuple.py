"""
Tool to create ROOT ntuple file from csv file. First line of text is used as column headers
txt2tuple input.txt output.root
some assumptions are made about calibGenCAL delimited TXT file conventions
"""

__facility__  = "Offline"
__abstract__  = "Tool to generate ROOT ntuple file from txt."
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/02/03 00:51:50 $"
__version__   = "$Revision: 1.9 $, $Author: fewtrell $"
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

    # setup logger
    logging.basicConfig()
    log = logging.getLogger('txt2tuple')
    log.setLevel(logging.INFO)

    # parse commandline
    if (len(sys.argv) != 3):
        log.error("Need 2 filenames: " + __doc__)
        sys.exit(1)
        
    inPath = sys.argv[1]
    outPath = sys.argv[2]

    ### open input 'csv' file ###
    csv_sample = file(inPath).read(8192)
    dialect = csv.Sniffer().sniff(csv_sample)
    has_header = csv.Sniffer().has_header(csv_sample)
    infile = csv.reader(open(inPath,"r"),dialect = dialect)


    outfile = ROOT.TFile(outPath, "RECREATE")
    # create ROOT File & Ntuple
    tuple_name = os.path.splitext(os.path.basename(outPath))[0]
    # strip bad characters
    tuple_name = re.compile('[. ]+').sub('_',tuple_name)
    # take first alpha string
    tuple_name = tuple_name.split('_')[0]

    tpl = None
    for line in infile:
        # first line init
        if tpl is None:
            # use optional header in first line for column names
            if has_header:
               header_cols = ":".join(line)
            else: # othewise just 'number' the columns
               header_cols = ":".join("_%s"%x for x in range(len(line)))

            # remove bad characters
            header_cols = header_cols.replace(";","")
            header_cols = header_cols.replace(",","")
            tpl = ROOT.TNtuple(tuple_name,
                               tuple_name,
                               header_cols)
            # don't process this line
            if has_header:
                continue

        vals = [float(x) for x in line]
        data = array.array('f',vals)
        tpl.Fill(data)


    log.info('Writing output file %s', outPath)

    outfile.Write()
    outfile.Close()

    sys.exit(0)                            
