#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/mergeDacSlopes.sh,v 1.2 2007/11/20 16:35:02 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/mergeDacSlopes.py "$@"

