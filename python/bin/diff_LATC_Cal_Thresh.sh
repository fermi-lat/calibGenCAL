#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/diff_LATC_CFE.sh,v 1.2 2007/11/20 16:35:02 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/diff_LATC_Cal_Thresh.py "$@"D
