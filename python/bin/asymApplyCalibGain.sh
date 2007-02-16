#! /bin/sh
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/asymApplyCalibGain.sh,v 1.3 2007/02/16 18:11:48 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/asymApplyCalibGain.py "$@"
