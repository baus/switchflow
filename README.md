## SwitchFlow

SwitchFlow is an experimental HTTP proxy in C++. I started the project in 2003 and was influenced by by Dan Kegel's 
[C10k](http://www.kegel.com/c10k.html) problem site. Because of this the server is single threaded, event driven 
and uses Niels Provos's [Libevent](http://libevent.org) 


## Quick build guide

```sh
sudo apt-get install libevent-dev libboost-all-dev cmake build-essential

source ./configbuild.sh

cd release/sfrp

make
```
