#! /bin/bash
#$Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/python/bin/dacSlopesXML2TXT.sh,v 1.2 2007/11/20 16:35:03 fewtrell Exp $
export -n DISPLAY

python ${CALIBGENCALROOT}/python/dacSlopesXML2TXT.py "$@"

