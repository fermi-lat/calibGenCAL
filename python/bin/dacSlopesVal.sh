#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/dacSlopesVal.sh,v 1.3 2007/02/16 18:11:48 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/dacSlopesVal.py "$@"

