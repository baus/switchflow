#!/bin/sh

dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

cd $dir/sfrp
cmake -DCMAKE_BUILD_TYPE:STRING=Debug

