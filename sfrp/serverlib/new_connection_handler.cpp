//
//Copyright (C) Christopher Baus.  All rights reserved.
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <util/Logger.h>
#include <util/PessimisticMemoryManager.h>
#include <event/event.hpp>

#include "new_connection_handler.hpp"
#include <iostream>

namespace serverlib{

new_connection_handler::new_connection_handler(PessimisticMemoryManager< i_connection_handler>* p_connection_handlers, 
                                               eventlib::poller* p_poller):
  p_connection_handlers_(p_connection_handlers),
  p_poller_(p_poller)
{
  
}
  

int new_connection_handler::handle_event(int fd, short revents, eventlib::event* pEvent)
{
  for(;;){
    std::cout<<"new connection"<<std::endl;
    int clientFd = accept_connection(fd);
    if(clientFd == -1){
      logDebug("Failed accepting client connection.  Assume no more clients ready to connect");
      break;
    }
    
    i_connection_handler* p_connection_handler = p_connection_handlers_->allocateElement();
    p_connection_handler->reset(clientFd);
    p_connection_handler->handle_event(clientFd, EV_READ|EV_WRITE, p_connection_handler->client_event_);
  }
  p_poller_->add_event(*pEvent, EV_READ|EV_WRITE, 0);

  return 0;
}  

int new_connection_handler::accept_connection(int socket)
{
  struct sockaddr_in peer_addr;
  socklen_t peer_len = sizeof(peer_addr);  
  return ::accept(socket, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len);
}

} //namespace proxylib

