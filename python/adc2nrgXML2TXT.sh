#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/adc2nrgXML2TXT.sh,v 1.1 2007/02/02 20:28:42 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/adc2nrgXML2TXT.py "$@"

