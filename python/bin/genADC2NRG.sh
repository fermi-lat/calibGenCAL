#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genADC2NRG.sh,v 1.2 2007/02/16 18:11:48 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/genADC2NRG.py "$@"
