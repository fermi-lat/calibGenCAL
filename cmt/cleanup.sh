tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if test ! $? = 0 ; then tempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt -quiet cleanup -sh -pack=calibGenCAL -version=v0 -path=/a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2 $* >${tempfile}; . ${tempfile}
/bin/rm -f ${tempfile}

