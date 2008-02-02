#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/dacSlopesTXT2XML.sh,v 1.2 2007/11/20 16:35:03 fewtrell Exp $
export -n DISPLAY

python ${CALIBGENCALROOT}/python/dacSlopesTXT2XML.py "$@"

