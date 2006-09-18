#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/dacXML2TXT.sh,v 1.1 2006/08/15 21:44:14 fewtrell Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:$ROOTSYS/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/dacXML2TXT.py "$@"
