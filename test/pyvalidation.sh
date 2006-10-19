#!/bin/sh
pushd $TALLAC/test/bin 
export PYTHONPATH=$TALLAC/test/bin/lib
python test.py
popd
