#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/biasTXT2XML.sh,v 1.1 2007/02/16 18:57:34 dwood Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/biasTXT2XML.py "$@"
