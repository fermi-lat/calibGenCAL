#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/muSlopeTXT2XML.sh,v 1.1 2006/06/28 22:08:02 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:$ROOTSYS/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/muSlopeTXT2XML.py "$@"
