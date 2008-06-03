#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/inlEval.sh,v 1.1 2008/04/29 15:46:36 fewtrell Exp $
export -n DISPLAY


python ${CALIBGENCALROOT}/python/inlEval.py "$@"

