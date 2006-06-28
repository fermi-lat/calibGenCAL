#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/adc2nrgTXT2XML.sh,v 1.3 2006/06/21 18:43:12 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/adc2nrgTXT2XML.py "$@"

