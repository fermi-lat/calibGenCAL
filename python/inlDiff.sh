#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/inlDiff.sh,v 1.1 2006/07/28 19:06:15 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/inlDiff.py "$@"
