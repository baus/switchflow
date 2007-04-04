//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <util/logger.hpp>
#include <util/pessimistic_memory_manager.hpp>
#include <event/event.hpp>

#include "new_connection_handler.hpp"
#include <iostream>

namespace serverlib{

new_connection_handler::new_connection_handler(pessimistic_memory_manager< i_connection_handler>* p_connection_handlers, 
                                               eventlib::poller* p_poller):
  p_connection_handlers_(p_connection_handlers),
  p_poller_(p_poller)
{
  
}
  

int new_connection_handler::handle_event(int fd, short revents, eventlib::event* p_event)
{
  for(;;){
    std::cout<<"new connection"<<std::endl;
    int client_fd = accept_connection(fd);
    if(client_fd == -1){
      log_debug("Failed accepting client connection.  Assume no more clients ready to connect");
      break;
    }
    
    i_connection_handler* p_connection_handler = p_connection_handlers_->allocate_element();
    p_connection_handler->reset(client_fd);
    p_connection_handler->handle_event(client_fd, EV_READ|EV_WRITE, p_connection_handler->client_event_);
  }
  p_poller_->add_event(*p_event, EV_READ|EV_WRITE, 0);

  return 0;
}  

int new_connection_handler::accept_connection(int socket)
{
  struct sockaddr_in peer_addr;
  socklen_t peer_len = sizeof(peer_addr);  
  return ::accept(socket, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len);
}

} //namespace proxylib

