#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/calFitsMerge.sh,v 1.3 2006/06/21 18:43:13 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/calFitsMerge.py "$@"
