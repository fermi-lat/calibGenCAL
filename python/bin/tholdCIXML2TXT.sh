#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/tholdCIXML2TXT.sh,v 1.5 2007/02/16 18:11:48 fewtrell Exp $
export -n DISPLAY

python ${CALIBGENCALROOT}/python/tholdCIXML2TXT.py "$@"
