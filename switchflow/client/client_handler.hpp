//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef CLIENT_HANDLERS_H__
#define CLIENT_HANDLERS_H__

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <list>

#include <boost/noncopyable.hpp>

#include <socketlib/status.hpp>
#include <socketlib/connection.hpp>
#include <event/i_event_handler.hpp>
#include <event/event.hpp>

#include <util/Pessimistic_memory_manager.h>
#include <util/read_write_buffer.hpp>

#include "i_client.hpp"
#include <util/read_write_buffer.hpp>
#include <string>
#include <memory>

namespace clientlib{
  
  class i_fp_monitor{
  public:
    virtual void add_fd(int fd) = 0;
    virtual void remove_fd(int fd) = 0;
  };

// client_handler handles notifications client proxy connections.  It maintains
// the state of the client socket handles.  
//
class client_handler: public eventlib::i_event_handler
{
 public:
  

  client_handler(eventlib::poller& poller,
                 unsigned int buffer_length, 
                 unsigned int server_timeout_seconds,
                 std::auto_ptr<i_client> p_client,
                 i_fp_monitor* p_fp_monitor = NULL);

  client_handler(const client_handler& rhs);
  

  virtual ~client_handler();
  
  virtual int handle_event(int fd, short revents, eventlib::event& p_event);

  client_handler* clone();

  //
  // This initiates the server connection.  The connection will not
  // be complete until a POLLOUT is received on the server socket handle.
  socketlib::STATUS initiate_server_connect(const sockaddr_in& server_addr);

  void dns_failed();

  i_client* get_client();

  socketlib::connection* get_socket_data();
  
 private:
  client_handler();
  void handle_timeout(int fd);
  
  void shutdown();

  void handle_pollin(int fd);

  void handle_pollout(int fd);  
  //
  // handles the completion of server socket connection.
  socketlib::STATUS handle_server_connect();

  socketlib::STATUS client_handler::check_for_server_connect(int fd);
  
  std::auto_ptr<i_client> p_client_;

  //
  // data for the server
  socketlib::connection server_data_;

  eventlib::event server_event_;

  unsigned int server_timeout_seconds_;
  
  eventlib::poller& poller_;

  socketlib::STATUS handle_stream();

  i_fp_monitor* p_fp_monitor_;
  };

}
#endif
