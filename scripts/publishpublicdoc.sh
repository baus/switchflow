#!/bin/sh
dir=$(dirname `echo $0 | sed -e "s,^\([^/]\),$(pwd)/\1,"`)

pushd $dir/../switchflow/
doxygen switchflow.dox
popd
rsync -r $dir/../switchflow/docs/html/ baus@baus.net:/var/www/bausnet/static/switchflow/docs

