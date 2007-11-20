#!/bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/fitNeighborXtalk.sh,v 1.1 2007/03/20 19:23:47 fewtrell Exp $

export -n DISPLAY
python ${CALIBGENCALROOT}/python/fitNeighborXtalk.py "$@"
