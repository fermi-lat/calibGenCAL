#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/xdiodeXtalk.sh,v 1.1 2006/08/03 13:11:03 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/xdiodeXtalk.py "$@"