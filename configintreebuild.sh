#!/bin/sh

##
## This creates the make files and dumps the object files in the source tree
## This is useful for debugging with GDB.

dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

cd $dir/sfrp
cmake -DCMAKE_BUILD_TYPE:STRING=Debug
cd $dir

