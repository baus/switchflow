//
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef NEW_CONNECTION_HANDLER_H__
#define NEW_CONNECTION_HANDLER_H__

#include <util/PessimisticMemoryManager.h>
#include <event/i_event_handler.hpp>
#include <event/poller.hpp>

#include "ProxyHandler.h"

namespace proxylib{

/**
 * Handles incoming connections from a server sockets.  
 */
class NewConnectionHandler: public eventlib::i_event_handler
{
 public:
  /**
   * Create a new new connection handler.  This is used wait for incoming connections
   * on a server socket.
   *
   * @param pProxyHandlers This is a pointer to ProxyHandlers that will
   *                        be attached to incoming connections.
   */
  NewConnectionHandler(PessimisticMemoryManager<ProxyHandler>* pProxyHandlers, 
                       PessimisticMemoryManager<IProxyStreamHandler>* pRequestStreamHandlers,
                       PessimisticMemoryManager<IProxyStreamHandler>* pResponseStreamHandlers,
                       eventlib::poller* pPoller);

  /**
   * Overrides the rnPoller::IEvent::notify() handler.  This will be 
   * called by the rnPoller framework.
   */
  virtual int handle_event(int fd, short revents, eventlib::event& event);
 
 private:
  // The client socket handlers that will be used when a connection has been
  // made.  
  PessimisticMemoryManager<ProxyHandler>* m_pProxyHandlers;
  
  
  PessimisticMemoryManager<IProxyStreamHandler>* m_pRequestStreamHandlers;
  PessimisticMemoryManager<IProxyStreamHandler>* m_pResponseStreamHandlers;
  
  //
  //
  int acceptClient(int proxyServerSocket);

  eventlib::poller* m_pPoller;
};

} // namespace proxylib

#endif // NEW_CONNECTION_HANDLER_H__
