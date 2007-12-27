#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/summarize_LATC_Cal_Thresh.sh,v 1.1 2007/12/27 21:54:34 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/summarize_LATC_Cal_Thresh.py "$@"
