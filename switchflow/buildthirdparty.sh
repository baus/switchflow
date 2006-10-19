#!/bin/sh

TALLAC=`pwd`/..

echo "building with TALLAC=$TALLAC"

if [ ! -e $TALLAC/thirdparty/lib/release/libevent.a ]; then
cd $TALLAC/thirdparty/libevent

export CFLAGS=-O2

./configure --prefix=$TALLAC/thirdparty/libevent/installdir\
            --libdir=$TALLAC/thirdparty/lib/release\
            --includedir=$TALLAC/thirdparty/include\
            --disable-shared
    
make clean        
make
make install

fi

if [ ! -e $TALLAC/thirdparty/lib/debug/libevent.a ]; then
cd $TALLAC/thirdparty/libevent

export CFLAGS=-g

./configure --prefix=$TALLAC/thirdparty/libevent/installdir\
            --libdir=$TALLAC/thirdparty/lib/debug\
            --includedir=$TALLAC/thirdparty/include\
            --disable-shared

make clean            
make
make install

fi
#
# build bjam.  bjam is boost's maker
#
OS=`uname -s`
echo $OS

if [ "$OS" = "Linux" ]; then
BJAM=$TALLAC/thirdparty/boost/tools/build/jam_src/bin.linuxx86/bjam
fi

if [ "$OS" = "FreeBSD" ]; then
BJAM=$TALLAC/thirdparty/boost/tools/build/jam_src/bin.freebsd/bjam
fi

if [ ! -e $BJAM ]; then
cd $TALLAC/thirdparty/boost/tools/build/jam_src
sh ./build.sh
fi


#
# now build boost
#
cd $TALLAC/thirdparty/boost
$BJAM --with-program_options --without-python

cp -f $TALLAC/thirdparty/boost/bin/boost/libs/program_options/build/libboost_program_options.a/gcc/debug/threading-multi/libboost_program_options-gcc-mt-d-1_33.a $TALLAC/thirdparty/lib/debug/libboost_program_options.a 
cp -f $TALLAC/thirdparty/boost/bin/boost/libs/program_options/build/libboost_program_options.a/gcc/release/threading-multi/libboost_program_options-gcc-mt-1_33.a $TALLAC/thirdparty/lib/release/libboost_program_options.a 

cd $TALLAC/switchflow
