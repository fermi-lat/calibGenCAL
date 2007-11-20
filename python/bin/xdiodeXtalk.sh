#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/xdiodeXtalk.sh,v 1.1 2007/02/16 18:57:36 dwood Exp $

export -n DISPLAY


python ${CALIBGENCALROOT}/python/xdiodeXtalk.py "$@"
