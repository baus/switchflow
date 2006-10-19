#!/bin/sh

pushd $TALLAC/sfrp/app
sh ./gensvnversion.sh
popd
pushd $TALLAC/release/sfrp
make clean
make
cd $TALLAC/release/sfrp/app
./sfrp --version
su root -c "killall sfrp-stage; \
       mv -f /usr/local/bin/sfrp-stage /usr/local/bin/sfrp-stage.bak; \
       cp ./sfrp /usr/local/bin/sfrp-stage; \
       sfrp-stage -f /etc/sfrp-stage.conf"
popd
