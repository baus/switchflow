#!/bin/sh

if [ ! -e $TALLAC/lib ]; then
mkdir $TALLAC/lib
cd $TALLAC/lib
ln -s `g++ -print-file-name=libstdc++.a`
fi

source $TALLAC/sfrp/app/gensvnversion.sh

mkdir -p $TALLAC/debug/switchflow
mkdir -p $TALLAC/debug/sfrp

cd $TALLAC/debug/switchflow
cmake ../../switchflow -DCMAKE_BUILD_TYPE:STRING=Debug

cd $TALLAC/debug/sfrp
cmake ../../sfrp -DCMAKE_BUILD_TYPE:STRING=Debug


mkdir -p $TALLAC/release/switchflow
mkdir -p $TALLAC/release/sfrp

cd $TALLAC/release/switchflow
cmake ../../switchflow -DCMAKE_BUILD_TYPE:STRING=Release

cd $TALLAC/release/sfrp
cmake ../../sfrp -DCMAKE_BUILD_TYPE:STRING=Release




