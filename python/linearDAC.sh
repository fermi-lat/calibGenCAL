#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/linearDAC.sh,v 1.1 2007/01/24 16:36:21 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/linearDAC.py "$@"
