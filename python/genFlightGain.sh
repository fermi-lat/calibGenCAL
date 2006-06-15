#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genFlightPed.sh,v 1.1 2006/06/12 19:39:41 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/genFlightGain.py "$@"


