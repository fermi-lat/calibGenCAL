#! /bin/sh
#$Header$

PYTHONPATH=${CALIBGENCALROOT}/python:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/get_slac_calibdac.py "$@"
