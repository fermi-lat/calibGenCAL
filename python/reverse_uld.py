#!/usr/local/bin/python

import calFitsXML
import calConstant
import Numeric
import sys


if __name__ == '__main__':

    # retrieve commandline parms
    inName  = sys.argv[1]
    outName = sys.argv[2]


    print "reverse_uld:"
    print "inFile:     ", inName
    print "outFile:    ", outName

    inFile = calFitsXML.calFitsXML(fileName = inName, mode = calFitsXML.MODE_READONLY)

    i = inFile.info()
    if i['TTYPE1'] != 'rng_uld_dac':
        print "file %s is not an ULD ADC file" % inName
        sys.exit(1)


    # read input file
    inData = inFile.read()
    info = inFile.info()
    tlist = inFile.getTowers()
    version = inFile.getVersion()

    print "inshape:    ", inData.shape

    # craete output array
    outData = Numeric.array(inData)
    print "outshape:   ", outData.shape

    # copy flipped data into output array
    for twr in tlist:
        for lyr in range(0,8):
            for col in range(0,12):
                for face in range(0,2):
                    for rng in range(0,3):
                        outData[rng][twr][lyr][face][col] = inData[rng][twr][lyr][face][col][::-1]


    # create output FITS file

    print "writing file %s", outName

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
        comment = info['COMMENT'], type = info['TTYPE1'], version = version)

    outFile.write(outData, tems = tlist)
    outFile.close()

    sys.exit(0)
