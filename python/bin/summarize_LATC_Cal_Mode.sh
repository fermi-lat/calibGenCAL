#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/summarize_LATC_Cal_Mode.sh,v 1.1 2007/12/20 17:36:46 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/summarize_LATC_CAL_Mode.py "$@"
