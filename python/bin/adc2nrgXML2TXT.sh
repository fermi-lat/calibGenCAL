#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/adc2nrgXML2TXT.sh,v 1.1 2007/02/16 18:57:34 dwood Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/adc2nrgXML2TXT.py "$@"

