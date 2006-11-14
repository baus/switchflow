//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// THIS SHOULD PROBABLY BE PUT INTO SERVERLIB
#ifndef NEW_CONNECTION_HANDLER_H__
#define NEW_CONNECTION_HANDLER_H__

#include <util/pessimistic_memory_manager.h>

#include <event/i_event_handler.hpp>
#include <event/poller.hpp>

#include "i_connection_handler.hpp"
namespace serverlib{

/**
 * Handles incoming connections from a server socket.  
 */
class new_connection_handler: public eventlib::i_event_handler
{
 public:
  new_connection_handler(pessimistic_memory_manager<i_connection_handler>*
                         p_connection_handlers, 
                         eventlib::poller* p_poller);

  /**
   * Overrides the rn_poller::i_event::notify() handler.  This will be 
   * called by the rn_poller framework.
   */
  virtual int handle_event(int fd, short revents, eventlib::event* p_event);
 
 private:
  // The client socket handlers that will be used when a connection has been
  // made.  
  pessimistic_memory_manager<i_connection_handler>* p_connection_handlers_;
  
  //
  //
  int accept_connection(int socket);

  eventlib::poller* p_poller_;
};

} // namespace proxylib

#endif // NEW_CONNECTION_HANDLER_H__
