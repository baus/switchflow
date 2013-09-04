## SwitchFlow HTTP Proxy

SwitchFlow is an experimental HTTP reverse proxy in C++. I started the project in 2003 and was influenced by by Dan Kegel's 
[C10k](http://www.kegel.com/c10k.html) problem. Because of this the server is single threaded, event driven 
and uses Niels Provos's [Libevent](http://libevent.org) for event aggregation.

The project was shelved in 2007 after Nginx gained traction, but I recently cleaned up the tree and got the proxy building 
on Ubuntu 12.04. 

### Quick build guide

```sh
sudo apt-get install libevent-dev libboost-all-dev cmake build-essential

source ./configbuild.sh

cd release/sfrp

make
```

### Configuring the proxy
The proxy uses a simple key/value pair configuration file. The configuration file also supports records and arrays of values. 
It is new line delimited.


