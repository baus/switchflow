//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.

//
// THIS SHOULD PROBABLY BE PUT INTO SERVERLIB
#ifndef NEW_CONNECTION_HANDLER_H__
#define NEW_CONNECTION_HANDLER_H__

#include <util/PessimisticMemoryManager.h>

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
  new_connection_handler(PessimisticMemoryManager<i_connection_handler>*
                         p_connection_handlers, 
                         eventlib::poller* p_poller);

  /**
   * Overrides the rnPoller::IEvent::notify() handler.  This will be 
   * called by the rnPoller framework.
   */
  virtual int handle_event(int fd, short revents, eventlib::event* pEvent);
 
 private:
  // The client socket handlers that will be used when a connection has been
  // made.  
  PessimisticMemoryManager<i_connection_handler>* p_connection_handlers_;
  
  //
  //
  int accept_connection(int socket);

  eventlib::poller* p_poller_;
};

} // namespace proxylib

#endif // NEW_CONNECTION_HANDLER_H__
