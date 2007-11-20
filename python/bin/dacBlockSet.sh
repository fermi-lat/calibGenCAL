#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/dacBlockSet.sh,v 1.1 2007/07/26 16:05:28 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/dacBlockSet.py "$@"
