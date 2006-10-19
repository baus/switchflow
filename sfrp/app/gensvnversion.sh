#/bin/sh
echo -n 'const char* SVNVersion(void) { const char* SVN_Version = "' > $TALLAC/sfrp/app/SVNVersion.cpp
svnversion -n $TALLAC/sfrp >> $TALLAC/sfrp/app/SVNVersion.cpp;
echo '"; return SVN_Version; }'  >> $TALLAC/sfrp/app/SVNVersion.cpp
