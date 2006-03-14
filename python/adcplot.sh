#! /bin/sh
#$Header$

PYTHONPATH=${CALIBGENCALROOT}/python:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/adcplot.py "$@"

