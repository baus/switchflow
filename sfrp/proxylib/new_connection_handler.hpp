//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef NEW_CONNECTION_HANDLER_H__
#define NEW_CONNECTION_HANDLER_H__

#include <util/pessimistic_memory_manager.hpp>
#include <event/i_event_handler.hpp>
#include <event/poller.hpp>

#include "proxy_handler.hpp"

namespace proxylib{

/**
 * Handles incoming connections from a server sockets.  
 */
class new_connection_handler: public eventlib::i_event_handler
{
 public:
  /**
   * Create a new new connection handler.  This is used wait for incoming connections
   * on a server socket.
   *
   * @param p_proxy_handlers This is a pointer to proxy_handlers that will
   *                        be attached to incoming connections.
   */
  new_connection_handler(pessimistic_memory_manager<proxy_handler>* p_proxy_handlers, 
                       pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handlers,
                       pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handlers,
                       eventlib::poller* p_poller);

  /**
   * Overrides the rn_poller::i_event::notify() handler.  This will be 
   * called by the rn_poller framework.
   */
  virtual int handle_event(int fd, short revents, eventlib::event& event);
 
 private:
  // The client socket handlers that will be used when a connection has been
  // made.  
  pessimistic_memory_manager<proxy_handler>* p_proxy_handlers_;
  
  
  pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handlers_;
  pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handlers_;
  
  //
  //
  int accept_client(int proxy_server_socket);

  eventlib::poller* p_poller_;
};

} // namespace proxylib

#endif // NEW_CONNECTION_HANDLER_H__
