set tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if $status != 0 then
  set tempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt -quiet cleanup -csh -pack=calibGenCAL -version=v0 -path=/a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2 $* >${tempfile}; source ${tempfile}
/bin/rm -f ${tempfile}

