# echo "Setting calibGenCAL v0 in /a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2"

CMTROOT=/afs/slac.stanford.edu/g/glast/applications/CMT/v1r12p20021129; export CMTROOT
. ${CMTROOT}/mgr/setup.sh

tempfile=`${CMTROOT}/mgr/cmt build temporary_name -quiet`
if test ! $? = 0 ; then tempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt -quiet setup -sh -pack=calibGenCAL -version=v0 -path=/a/sulky07/g.glast.u06/chen/glast_mc/GlastRelease_v3r3p2 $* >${tempfile}; . ${tempfile}
/bin/rm -f ${tempfile}

