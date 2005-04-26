"""
Tool to convert CAL FITS files to XML format.
"""


__facility__  = "Offline"
__abstract__  = "Tool to convert CAL FITS files to XML format"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/12 14:06:47 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name: HEAD $"
__credits__   = "NRL code 7650"


import sys
import Numeric
import logging
import calFitsIO, calFitsXML

from calConstant import CFACE


# setup logger

logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)


# check command line

if len(sys.argv) != 3:
    log.error("usage: fits2xml <fits_file> <xml_file>")
    sys.exit(-1)
fitsName = sys.argv[1]
xmlName = sys.argv[2]


# open and read FITS file

log.info("fits2xml: reading FITS file %s", fitsName)

fio = calFitsIO.calFitsIO(fileName = fitsName, mode = calFitsIO.MODE_READONLY)
info = fio.info()
k = info.keys()
k.sort()
for i in k:
    log.debug('fits2xml: %-16s:  %s', i, str(info[i]))

data = fio.read()
fio.close()


# create new XML document

log.info("fits2xml: creating XML file %s", xmlName)

sn = {}
for n in CFACE:
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


xmlFile = calFitsXML.calFitsXML(fileName = xmlName, mode = calFitsXML.MODE_CREATE, labels = la, \
    calSNs = sn, dataset = info['DATASET'], lrefgain = info['LREFGAIN'], hrefgain = info['HREFGAIN'], \
    pedFile = info['PEDFILE'], erng = info['ERNG'], fitsName = info['FITSNAME'], fitsTime = info['FITSTIME'], \
    reportName = info['RPTNAME'], runId = info['RUNID'], comment = info['COMMENT'])

xmlFile.write(data)
xmlFile.close()

sys.exit(0)

