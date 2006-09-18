#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/build_gensettings_cfg.sh,v 1.3 2006/06/21 18:43:12 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:$ROOTSYS/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/build_gensettings_cfg.py "$@"
