#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/muSlopeDiff.sh,v 1.1 2006/09/07 21:35:59 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:$ROOTSYS/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/muSlopeDiff.py "$@"
