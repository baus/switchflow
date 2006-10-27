//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

/** 
 * Copyright (C) Christopher Baus.  All rights reserved.
 */
#include <util/logger.hpp>
#include "NewConnectionHandler.h"

namespace proxylib{

NewConnectionHandler::NewConnectionHandler(PessimisticMemoryManager<ProxyHandler>* pProxyHandlers,
                                           PessimisticMemoryManager<IProxyStreamHandler>* pRequestStreamHandlers,
                                           PessimisticMemoryManager<IProxyStreamHandler>* pResponseStreamHandlers,
                                           eventlib::poller* pPoller):
  pProxyHandlers_(pProxyHandlers), 
  pRequestStreamHandlers_(pRequestStreamHandlers),
  pResponseStreamHandlers_(pResponseStreamHandlers),
  pPoller_(pPoller)
{
}
  

int NewConnectionHandler::handle_event(int fd, short revents, eventlib::event& event)
{
  socketlib::STATUS status = socketlib::COMPLETE;
  for(;;){
    int clientFd = acceptClient(fd);
    if(clientFd == -1){
      break;
    }
    //
    // Never need to deallocate instance.  Ownership is transfered
    // to the instance itself.  When it is shutdown, it releases itself.
    // That is why pProxyHandlers_ is passed to initialize.
    //
    ProxyHandler* pProxyHandler = pProxyHandlers_->allocateElement();

    pProxyHandler->reset(pProxyHandlers_, pRequestStreamHandlers_, pResponseStreamHandlers_);
    status = pProxyHandler->addClient(clientFd);
    //
    // If the stream does not provide JIT connection, connect right now.
    if(status == socketlib::COMPLETE &&
       pProxyHandler->pRequestStreamHandler_->getForwardAddressStatus() ==
       IProxyStreamHandler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS){
      
      pProxyHandler->initiateServerConnect(pProxyHandler->serverAddr_);      
    }

    //
    // Must assume the client is ready to read after it connects.
#warning should we attempt to read?    
//    pProxyHandler->handle_event(clientFd, EV_READ);
  }
  pPoller_->add_event(event, EV_READ|EV_WRITE, 0);
  return 0;
}  

int NewConnectionHandler::acceptClient(int proxyServerSocket)
{
  struct sockaddr_in peer_addr;
  socklen_t peer_len = sizeof(peer_addr);  
  return ::accept(proxyServerSocket, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len);
}

} //namespace proxylib

