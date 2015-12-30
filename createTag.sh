VERSION=`grep INSTAMSG_VERSION common/instamsg/driver/include/globals.h | cut -d\  -f 3 | cut -d\" -f 2`
git tag ${VERSION}
git push
git push --tags
