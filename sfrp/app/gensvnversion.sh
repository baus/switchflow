#!/bin/sh
dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)
echo -n 'const char* SVNVersion(void) { const char* SVN_Version = "' > $dir/SVNVersion.cpp
svnversion -n $dirname/.. >> $dir/SVNVersion.cpp;
echo '"; return SVN_Version; }'  >> $dir/SVNVersion.cpp
