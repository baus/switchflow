#!/bin/sh
killall thttpd
killall ssvs
cd $TALLAC/test
echo "Starting origin server"
bin/thttpd -nor -d $TALLAC/test/www -l $TALLAC/test/thttpdlog -p 10000

echo "Starting validation server"
bin/ssvs -f config.xml > out&

