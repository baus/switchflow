//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus. All Rights Reserved
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <signal.h>
#include <pwd.h>

#include <string>
#include <iostream>

#include <asio.hpp>

#include <util/ScopeGuard.h>
#include <util/logger.hpp>
#include <util/read_write_buffer.hpp>
#include <util/PessimisticMemoryManager.h>
#include <util/config_file.hpp>

#include <http/http.hpp>
#include <proxylib/ProxyHandler.h>
#include <proxylib/NewConnectionHandler.h>

#include <http/header_handler.hpp>
#include <http/header_cache.hpp>

#include <event/poller.hpp>

#include "host_map.hpp"
#include "HTTPProxyStreamHandler.h"
#include "NonValidatingProxyStreamHandler.h"
#include "Options.h"
#include "AccessLog.hpp"
#include "pipeline_data_factory.hpp"


bool daemonize(const config_file& config)
{
  bool daemonize = config["proxy"]["run-as-daemon"].read<bool>();
  if(daemonize){
    log_info("starting daemon");
    if(::daemon(0, 0) != 0){
      log_error("failed to start as daemon");
      return false;
    }
  }
  return true;
}

bool switch_user(const config_file& config)
{
  struct passwd *pw;
  std::string username = config["proxy"]["user"].read<std::string>();
  
  if (username.length() == 0) {
    log_error("No user provided.  Refusing to run as root.");
    return false;
  }
  if ((pw = getpwnam(username.c_str())) == 0) {
    log_error("Could not find specified user.");
    return false;
  }
  if(pw->pw_uid == 0){
    log_error("Refusing to run as root.  Please use less privileged user.");
    return false;
  }
  if (setgid(pw->pw_gid)<0 || setuid(pw->pw_uid)<0) {
    log_error("Failed to become specified user.");
    return false;
  }
  return true;
}



int main(int argc, char *argv[])
{
  typedef char* mytype;
  mytype array2;
  char array[sizeof(int)];
  
  ::signal(SIGPIPE, SIG_IGN);
  
  logger_init("sfrp");
  ON_BLOCK_EXIT(logger_shutdown);
  
  Options options;
  Options::STATUS optionsStatus = options.processCommandLine(argc, argv);
  
  if(optionsStatus == Options::ERROR){
    return -1;
  }
  else if(optionsStatus == Options::INFO){
    return 0;
  }
  assert(optionsStatus == Options::RUN);
  
  
  config_file config;
  config.parse_file(options.getConfigFile().c_str());
  
  http::header_cache headers(config["http-parser"]["header-pool-size"].read<int>(),
                             config["http-parser"]["max-header-name-length"].read<int>(),
                             config["http-parser"]["max-header-value-length"].read<int>());

  AccessLog access_log;
  
  if(config["access-log"]["enable"].read<bool>()){
    if(!access_log.open(config["access-log"]["location"].read<std::string>().c_str())){
      log_info("Failed to open access log file.");
    }
  }

  asio::ip::address bind_addr = asio::ip::address::from_string(config["proxy"]["listen-address"][(size_t)0]["address"].read<std::string>());
  unsigned short bind_port = config["proxy"]["listen-address"][(size_t)0]["port"].read<unsigned short>();
  asio::ip::tcp::endpoint bind_point(bind_addr, bind_port);
  
  
  int server_handle;
  
  if ((server_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    log_error("Socket() failed while creating server socket");
    return -1;
  }
  ON_BLOCK_EXIT(::close, server_handle); 



  //
  // Set the socket option SO_REUSEADDR so users can re-bind to the port right away
  int val=1;
  ::setsockopt(server_handle, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
  if (::bind(server_handle, bind_point.data(), sizeof(asio::ip::tcp::endpoint::data_type)) == -1) {
    log_error("Failed to bind server socket.\n\tDoes the current user have privledge to open server sockets?\n\tIs another processing listening on the requested port?");
    return -1;
  }
  
  http::init();
    
  //
  // Entering run stage
  if (::listen(server_handle, 100) == -1) {
    log_error("listen() failed while creating server socket");
    return -1;
  }

  //
  // switch user now that we are listening
  if(!switch_user(config)){
    return -1;
  }
  
  //
  // daemonize now that we are accepting connections.
  if(!daemonize(config)){
    return -1;
  }

  //
  // fire up libevent
  eventlib::poller poller;

  asio::ip::address forward_addr = asio::ip::address::from_string(config["proxy"]["default-forward-address"]["address"].read<std::string>());
  unsigned short forward_port = config["proxy"]["default-forward-address"]["port"].read<unsigned short>();
  asio::ip::tcp::endpoint forward_point(forward_addr, forward_port);


  
  
  PessimisticMemoryManager<proxylib::ProxyHandler> 
    proxyHandlers(config["proxy"]["max-connections"].read<unsigned int>(), 
                  proxylib::ProxyHandler(&poller,
                                         config["proxy"]["input-buffer-size"].read<unsigned int>(),
                                         config["proxy"]["client-timeout-milliseconds"].read<unsigned int>(),
                                         config["proxy"]["server-timeout-milliseconds"].read<unsigned int>(),
                                         *forward_point.data(),
                                         &create_pipeline_data));

  

  std::map<std::string, std::pair<httplib::URL, bool> > host_map;

  build_host_map(config, host_map);
  
  //
  // This is where most of our memory gets allocated.  This is the key to
  // the memory strategy for the server core.  If memory is need by a 
  // connection is should be added to the BufferManager.
  HTTPProxyStreamHandler requestStreamPrototype( &headers,
                                                 host_map,
                                                 http::REQUEST,
                                                 http::max_method_length(),
                                                 config["http-parser"]["max-URI-length"].read<unsigned int>(),
                                                 http::max_version_length, // length of the version.  Only accept 8 characters
                                                 config["http-parser"]["max-num-headers"].read<unsigned int>(),

                                                 config["http-parser"]["max-header-name-length"].read<unsigned int>(),
                                                 config["http-parser"]["max-header-value-length"].read<unsigned int>(),
                                              
                                                 &access_log,
//                                                 &rb_processor);
                                                 0);
  
  HTTPProxyStreamHandler responseStreamPrototype( &headers,
                                                  host_map,
                                                  http::RESPONSE,
                                                  http::max_version_length,
                                                  config["http-parser"]["max-URI-length"].read<unsigned int>(),
                                                  50, 
                                                  config["http-parser"]["max-num-headers"].read<unsigned int>(),
                                                  config["http-parser"]["max-header-name-length"].read<unsigned int>(),
                                                  config["http-parser"]["max-header-value-length"].read<unsigned int>(),
                                                  &access_log,
//                                                  &rb_processor);
                                                  0);
  


  PessimisticMemoryManager<proxylib::IProxyStreamHandler> 
    requestStreamHandlers(config["proxy"]["max-connections"].read<unsigned int>(),
                          static_cast<proxylib::IProxyStreamHandler*>(&requestStreamPrototype));
  
  PessimisticMemoryManager<proxylib::IProxyStreamHandler> 
    responseStreamHandlers(config["proxy"]["max-connections"].read<unsigned int>(),
                           static_cast<proxylib::IProxyStreamHandler*>(&responseStreamPrototype));

  proxylib::NewConnectionHandler newConnectionHandler(&proxyHandlers,
                                                      &requestStreamHandlers,
                                                      &responseStreamHandlers,
                                                      &poller);

  eventlib::event newConnectionEvent;
  newConnectionEvent.set(server_handle, &newConnectionHandler);
  poller.add_event(newConnectionEvent, EV_READ|EV_WRITE, 0);


  //
  // now run the server.
  poller.dispatch();
  
  log_info("Shutting down.");
  //
  // A lot destructors and ScopeGuards are about to go off here.
  //
#warning not cleaning exiting when signals are fired.  Need to figure out libevent signal handling
}
