#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/xdiodeXtalk.sh,v 1.1 2006/10/03 21:12:24 fewtrell Exp $

export -n DISPLAY


python ${CALIBGENCALROOT}/python/xdiodeXtalk.py "$@"
