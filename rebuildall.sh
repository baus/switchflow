#!/bin/sh
dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

cd $dir/release/sfrp
make clean
make

cd $dir/debug/sfrp
make clean
make
