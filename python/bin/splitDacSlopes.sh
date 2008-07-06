#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/splitDacSlopes.sh,v 1.1 2008/02/02 23:25:39 fewtrell Exp $
export -n DISPLAY

python ${CALIBGENCALROOT}/python/splitDacSlopes.py "$@"

