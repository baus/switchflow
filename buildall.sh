#!/bin/sh
dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

rm -f $dir/thirdparty/lib/debug/*
rm -f $dir/thirdparty/lib/release/*

$dir/switchflow/buildthirdparty.sh
$dir/configbuild.sh

cd $dir/release/sfrp
make clean
make

cd $dir/debug/sfrp
make clean
make
