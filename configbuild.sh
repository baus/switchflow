#!/bin/sh

dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

source $dir/sfrp/app/gensvnversion.sh

mkdir -p $dir/debug/sfrp

cd $dir/debug/sfrp
cmake ../../sfrp -DCMAKE_BUILD_TYPE:STRING=Debug


mkdir -p $dir/release/sfrp

cd $dir/release/sfrp
cmake ../../sfrp -DCMAKE_BUILD_TYPE:STRING=Release

cd $dir


