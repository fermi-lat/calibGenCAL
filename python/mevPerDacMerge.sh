#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/mevPerDacMerge.sh,v 1.4 2006/09/18 20:19:20 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/mevPerDacMerge.py "$@"
