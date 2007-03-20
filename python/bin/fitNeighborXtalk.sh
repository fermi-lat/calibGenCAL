#!/bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/fitNeighborXtalk.sh,v 1.1 2007/02/16 18:57:34 dwood Exp $

export -n DISPLAY
python ${CALIBGENCALROOT}/python/fitNeighborXtalk.py "$@"
