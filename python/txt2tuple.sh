#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/txt2tuple.sh,v 1.1 2007/01/12 18:49:11 fewtrell Exp $
export -n DISPLAY



python ${CALIBGENCALROOT}/python/txt2tuple.py "$@"
