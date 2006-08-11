#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/pedDiff.sh,v 1.1 2006/08/10 18:06:43 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/pedDiff.py "$@"
