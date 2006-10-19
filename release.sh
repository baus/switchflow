#!/bin/sh

pushd $TALLAC/sfrp/app
sh ./gensvnversion.sh
popd
source ./buildtarfile.sh

# use cp rather than mv to get the correct permissions in the 
# release directory
cp -f $TARFILE /var/www/bausnet/static/switchflow-reverse-proxy;rm -f $TARFILE


