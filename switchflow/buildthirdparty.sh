#!/bin/sh
dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

TALLAC=$dir/..

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

GCC_VERSION=`gcc -dumpversion`
cd $TALLAC/thirdparty/boost

./configure --prefix=$TALLAC/thirdparty/libevent/installdir\
            --with-libraries=program_options

make clean
make

cp $TALLAC/thirdparty/boost/bin.v2/libs/program_options/build/gcc-$GCC_VERSION/debug/link-static/libboost_program_options*.a $TALLAC/thirdparty/lib/debug/libboost_program_options.a

cp $TALLAC/thirdparty/boost/bin.v2/libs/program_options/build/gcc-$GCC_VERSION/release/link-static/libboost_program_options*.a $TALLAC/thirdparty/lib/release/libboost_program_options.a

