# echo "Setting calibGenCAL v0 in /a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2"

setenv CMTROOT /afs/slac.stanford.edu/g/glast/applications/CMT/v1r12p20021129
source ${CMTROOT}/mgr/setup.csh

set tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt -quiet setup -csh -pack=calibGenCAL -version=v0 -path=/a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2 $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}

