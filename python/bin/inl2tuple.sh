#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/inl2tuple.sh,v 1.1 2007/02/16 18:57:35 dwood Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/inl2tuple.py "$@"
