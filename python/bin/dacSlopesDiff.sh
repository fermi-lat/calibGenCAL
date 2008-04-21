#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/dacSlopesDiff.sh,v 1.1 2008/04/21 14:37:04 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/dacSlopesDiff.py "$@"

