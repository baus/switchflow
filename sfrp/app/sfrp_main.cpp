//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <signal.h>
#include <pwd.h>

#include <string>
#include <iostream>

#include <asio.hpp>

#include <util/scope_guard.hpp>
#include <util/logger.hpp>
#include <util/read_write_buffer.hpp>
#include <util/pessimistic_memory_manager.hpp>
#include <util/config_file.hpp>

#include <http/http.hpp>
#include <proxylib/proxy_handler.hpp>
#include <proxylib/new_connection_handler.hpp>

#include <http/header_handler.hpp>
#include <http/header_cache.hpp>

#include <event/poller.hpp>

#include "host_map.hpp"
#include "http_proxy_stream_handler.hpp"
#include "options.hpp"
#include "access_log.hpp"
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
  
  options options;
  options::STATUS options_status = options.process_command_line(argc, argv);
  
  if(options_status == options::ERROR){
    return -1;
  }
  else if(options_status == options::INFO){
    return 0;
  }
  CHECK_CONDITION(options_status == options::RUN, "invalid option status");
  
  
  config_file config;
  config.parse_file(options.get_config_file().c_str());
  
  http::header_cache headers(config["http-parser"]["header-pool-size"].read<int>(),
                             config["http-parser"]["max-header-name-length"].read<int>(),
                             config["http-parser"]["max-header-value-length"].read<int>());

  access_log access_log;
  
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
    log_error("Failed to bind server socket.\n\t_does the current user have privledge to open server sockets?\n\t_is another processing listening on the requested port?");
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


  
  
  pessimistic_memory_manager<proxylib::proxy_handler> 
    proxy_handlers(config["proxy"]["max-connections"].read<unsigned int>(), 
                  proxylib::proxy_handler(&poller,
                                         config["proxy"]["input-buffer-size"].read<unsigned int>(),
                                         config["proxy"]["client-timeout-milliseconds"].read<unsigned int>(),
                                         config["proxy"]["server-timeout-milliseconds"].read<unsigned int>(),
                                         *forward_point.data(),
                                         &create_pipeline_data));

  

  std::map<std::string, std::pair<httplib::url, bool> > host_map;

  build_host_map(config, host_map);
  
  //
  // This is where most of our memory gets allocated.  This is the key to
  // the memory strategy for the server core.  If memory is needed by a 
  // connection it should be added to the buffer_manager.
  http_proxy_stream_handler request_stream_prototype( &headers,
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
  
  http_proxy_stream_handler response_stream_prototype( &headers,
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
  


  pessimistic_memory_manager<proxylib::i_proxy_stream_handler> 
    request_stream_handlers(config["proxy"]["max-connections"].read<unsigned int>(),
                          static_cast<proxylib::i_proxy_stream_handler*>(&request_stream_prototype));
  
  pessimistic_memory_manager<proxylib::i_proxy_stream_handler> 
    response_stream_handlers(config["proxy"]["max-connections"].read<unsigned int>(),
                           static_cast<proxylib::i_proxy_stream_handler*>(&response_stream_prototype));

  proxylib::new_connection_handler new_connection_handler(&proxy_handlers,
                                                      &request_stream_handlers,
                                                      &response_stream_handlers,
                                                      &poller);

  eventlib::event new_connection_event;
  new_connection_event.set(server_handle, &new_connection_handler);
  poller.add_event(new_connection_event, EV_READ|EV_WRITE, 0);


  //
  // now run the server.
  poller.dispatch();
  
  log_info("Shutting down.");
  //
  // A lot destructors and scope_guards are about to go off here.
  //
#warning not cleanly exiting when signals are fired.  Need to figure out libevent signal handling
}
