#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/diff_LATC_Cal_Thresh.sh,v 1.1 2007/12/20 00:36:51 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/diff_LATC_Cal_Thresh.py "$@"
