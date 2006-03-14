#! /bin/sh
#$Header$

PYTHONPATH=${CALIBGENCALROOT}/python:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/asymXML2TXT.py "$@"

