#!/bin/sh


cd $TALLAC/sfrp
cmake -DCMAKE_BUILD_TYPE:STRING=Debug
cd $TALLAC/crawler
cmake -DCMAKE_BUILD_TYPE:STRING=Debug

