#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/genADC2NRG.sh,v 1.2 2006/09/18 20:19:20 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:$ROOTSYS/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/genADC2NRG.py "$@"
