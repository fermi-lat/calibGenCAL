#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/summarize_LATC_Cal_Thresh.sh,v 1.2 2007/12/21 17:42:10 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/summarize_LATC_CAL_Thresh.py "$@"
