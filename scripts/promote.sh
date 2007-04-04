#!/bin/sh

su -l root -c "killall sfrp; \
       cp -f /usr/local/bin/sfrp /usr/local/bin/sfrp.bak; \
       cp -f /usr/local/bin/sfrp-stage /usr/local/bin/sfrp; \
       sfrp --version; \
       sfrp -f /etc/sfrp.conf"
