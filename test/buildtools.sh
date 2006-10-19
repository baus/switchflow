#!/bin/sh
#
# we will put our binaries used for the test here.
#
if [ ! -e $TALLAC/test/bin ]; then
    mkdir $TALLAC/test/bin
fi

#
# build flood.  We will use flood for some stress testing
#
#cd $TALLAC/test/flood
#make clean
#./configure --prefix=$TALLAC/test/bin --bindir=$TALLAC/test/bin
#make
#cp $TALLAC/test/flood/flood $TALLAC/test/bin

#
# build thttpd. thttpd is small and fast.  We'll use that
# as our origin server.  
#
cd $TALLAC/test/thttpd
make clean
./configure --prefix=$TALLAC/test --bindir=$TALLAC/test
make
cp $TALLAC/test/thttpd/thttpd $TALLAC/test/bin

