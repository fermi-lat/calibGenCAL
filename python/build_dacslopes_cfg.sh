#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/build_tholdci_cfg.sh,v 1.2 2006/06/21 18:43:12 dwood Exp $

PYTHONPATH=${CALIBGENCALROOT}/python/lib:${PYTHONPATH}
export PYTHONPATH

python ${CALIBGENCALROOT}/python/build_dacslopes_cfg.py "$@"
