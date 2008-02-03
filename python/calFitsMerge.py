"""
Tool to merge mutilple single tower calFitsXML.py files into a
single output file.

Currently Supported file types are:
- relgain

The command line is:

calFitsMerge [-V] <output_xml_file> <input_xml_file1> [input_xml_fileN]*

where:
    -V                = verbose; turn on debug output
    <out_xml_file>    = The merged XML file to output.
    <input_xml_files> = space delimited list of input xmls files
"""


__facility__  = "Offline"
__abstract__  = "Tool to merge mutilple calFitsXML files."
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/06/27 19:30:16 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os
import logging
import getopt
import re

import Numeric

import calFitsXML
import calConstant

def calsn_name(face):
    return "CALSN%s"%face

#######################################################################################

if __name__ == '__main__':


    # setup logger

    logging.basicConfig()
    log = logging.getLogger('calFitsMerge')
    log.setLevel(logging.INFO)


    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-V")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-V':
            log.setLevel(logging.DEBUG)        

    args = opts[1]
    if len(args) < 2:
        log.error(__doc__)
        sys.exit(1)

    outFilename = args.pop(0)

    # get input filenames
    inFilenames = args

    ## HEADER LISTS ##
    # certain header fields will end up
    # being lists of values from
    # each input file
    fitsname   = []
    fitstime   = []
    dataset    = []
    pedfile    = []
    runid      = []
    rptname    = []
    instrument = []
    # a dict of lists
    calsn  = {}
    for face in calConstant.CFACE:
        calsn[face] = []

    #########################
    # read first input file #
    #########################
    inFilename = inFilenames.pop(0)
    log.info("Opening %s for read"%inFilename)
    inFile = calFitsXML.calFitsXML(fileName=inFilename)

    # get header info from first file
    # base output file header on this info
    outfile_hdr  = inFile.info()
    outfile_twrs = set(inFile.getTowers())
    outfile_data = inFile.read()

    inFile.close()

    ## header lists ##
    fitsname.append(outfile_hdr['FITSNAME'])
    fitstime.append(outfile_hdr['FITSTIME'])
    dataset.append(outfile_hdr['DATASET'])
    pedfile.append(outfile_hdr['PEDFILE'])
    runid.append(outfile_hdr['RUNID'])
    rptname.append(outfile_hdr['RPTNAME'])
    instrument.append(outfile_hdr['INSTRUMENT'])
    for face in calConstant.CFACE:
        calsn[face].append(outfile_hdr[calsn_name(face)])

    ##########################
    # merge subsequent files #
    ##########################
    for inFilename in inFilenames:
        log.info("Opening %s for read"%inFilename)
        inFile = calFitsXML.calFitsXML(fileName=inFilename)
        infile_hdr  = inFile.info()
        infile_twrs = set(inFile.getTowers())
        infile_data = inFile.read()

        ## VALIDATE ##
        if infile_hdr['LREFGAIN'] != outfile_hdr['LREFGAIN'] or \
           infile_hdr['HREFGAIN'] != outfile_hdr['HREFGAIN']:
            log.error("gains do not match")
            sys.exit(1)
        

        # module ids should not repeat
        if len(outfile_twrs & infile_twrs) > 0:
            log.error("tower info repeated: ")
            sys.exit(1)
        outfile_twrs = outfile_twrs | infile_twrs

        ## header lists ##
        fitsname.append(infile_hdr['FITSNAME'])
        fitstime.append(infile_hdr['FITSTIME'])
        dataset.append(infile_hdr['DATASET'])
        pedfile.append(infile_hdr['PEDFILE'])
        runid.append(infile_hdr['RUNID'])
        rptname.append(infile_hdr['RPTNAME'])
        instrument.append(infile_hdr['INSTRUMENT'])
        for face in calConstant.CFACE:
            calsn[face].append(infile_hdr[calsn_name(face)])


        ## MERGE ##
        for twr in infile_twrs:
            outfile_data[:,:,twr,:] = infile_data[:,:,twr,:]


    ## WRITE OUTPUT FILE ##

    # create 'labels' header array
    """ (from calFitsXML doc)
            Param: labels A sequence of FITS table header values:
            labels[0] = Table header \e LAXIS1 string value
            labels[1] = Table header \e LAXIS2 string value
            ...
            labels[-2] = Table header \e LAXIS<n> string value.
            labels[-1] = Table header \e TTYPE1 string value.
    """
    axis_re = re.compile('^LAXIS')
    axes = [axis for axis in outfile_hdr.keys() \
            if (axis_re.match(axis) and outfile_hdr[axis] != None)]
    axes.sort()
    labels = []
    for axis in axes:
        labels.append(outfile_hdr[axis])
    labels.append(outfile_hdr['TTYPE1'])
    
    # convert calsn lists to strings
    for face in calConstant.CFACE:
        calsn[face] = str(calsn[face])
            
    log.info("Opening %s for write" % outFilename)
    output_file = calFitsXML.calFitsXML(fileName = outFilename,
                                        mode     = calFitsXML.MODE_CREATE,
                                        labels   = labels,
                                        type     = outfile_hdr['TTYPE1'],
                                        lrefgain = outfile_hdr['LREFGAIN'],
                                        hrefgain = outfile_hdr['HREFGAIN'],
                                        fitsName   = str(fitsname),
                                        fitsTime   = str(fitstime),
                                        dataset    = str(dataset),
                                        pedFile    = str(pedfile),
                                        runId      = str(runid),
                                        reportName = str(rptname),
                                        instrument = str(instrument),
                                        calSNs     = calsn
                                        )
    output_file.write(outfile_data, list(outfile_twrs))

    
        

