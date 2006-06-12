#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genFHEsettings.sh,v 1.2 2006/03/14 22:42:44 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/genFlightPed.py "$@"


