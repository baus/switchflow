## SwitchFlow HTTP Proxy

[View on GitHub](http://github.com/baus/switchflow)

SwitchFlow is an experimental HTTP reverse proxy in C++. [I](http://baus.net/) started the project in 2003 and was influenced by Dan Kegel's 
[C10k](http://www.kegel.com/c10k.html) problem. Because of this the server is single threaded, event driven 
and uses Niels Provos's [Libevent](http://libevent.org) for event aggregation.

The project was shelved in 2007 after Nginx gained traction, but I recently cleaned up the tree and got the proxy building 
on Ubuntu 12.04. 

It can be used to configure virtual hosts and send requests to multiple origin servers. 

### Quick build guide

```sh
# Install dependencies
sudo apt-get install libevent-dev libboost-all-dev cmake build-essential
# run script to configure build environment using cmake
source ./configbuild.sh
# build the proxy
cd release/sfrp
make
```

### Configuring the proxy
The proxy uses a simple key/value pair configuration file. The configuration file also supports records and arrays of values. 
It is new line delimited. An example configuration file included in the tree at /sfrp/app/sfrp.conf.example.

```
# Directs the proxy to detach from the console and run as a daemon
proxy.run-as-daemon=no

# This is the address the proxy will listen on
proxy.listen-address[0].address=127.0.0.1
proxy.listen-address[0].port=8080

# This is the address the proxy will forward requests to
# by default.
proxy.default-forward-address.address=204.232.175.78
proxy.default-forward-address.port=80

# Enable apache combined style access logging.
access-log.enable=yes
access-log.location=/var/log/sfrp_access_log_stage

# The following shows how to describe virtual hosts

# host-name is the value sent by the client in the host HTTP header.
virtual-host[0].host-name=localhost:8080

# This is the URL to send requests which match the example.com host
# to.  The specified URL path is prepended to incoming requests' paths.
virtual-host[0].default-forward-url=http://baus.net

# This tells the proxy to send the request's host to server, rather than
# replace the host with server's host.  In this case 127.0.0.1 will
# receive a host of example.com.  In other cases it might be useful
# to pass the name 
virtual-host[0].preserve-host=false


# The following are used to performance tune
# the HTTP parser's memory usage.  You probably
# don't need to change these unless you have
# specific requirements.
http-parser.max-method-length=256
http-parser.max-URI-length=512
http-parser.max-num-headers=50
http-parser.max-header-name-length=256
http-parser.max-header-value-length=512
http-parser.header-pool-size=75000
proxy.input-buffer-size=2000

#
# The user name to switch to after starting the proxy and binding to
# server socket.
proxy.user=nobody

#
# The following configure the proxy's connection
# handling.
proxy.client-timeout-milliseconds=30000
proxy.server-timeout-milliseconds=10000
proxy.max-connections=1000
```
