//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/logger.hpp>
#include "new_connection_handler.hpp"

namespace proxylib{

new_connection_handler::new_connection_handler(pessimistic_memory_manager<proxy_handler>* p_proxy_handlers,
                                           pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handlers,
                                           pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handlers,
                                           eventlib::poller* p_poller):
  p_proxy_handlers_(p_proxy_handlers), 
  p_request_stream_handlers_(p_request_stream_handlers),
  p_response_stream_handlers_(p_response_stream_handlers),
  p_poller_(p_poller)
{
}
  

int new_connection_handler::handle_event(int fd, short revents, eventlib::event& event)
{
  socketlib::STATUS status = socketlib::COMPLETE;
  for(;;){
    int client_fd = accept_client(fd);
    if(client_fd == -1){
      break;
    }
    //
    // Never need to deallocate instance.  Ownership is transfered
    // to the instance itself.  When it is shutdown, it releases itself.
    // That is why p_proxy_handlers_ is passed to initialize.
    //
    proxy_handler* p_proxy_handler = p_proxy_handlers_->allocate_element();

    p_proxy_handler->reset(p_proxy_handlers_, p_request_stream_handlers_, p_response_stream_handlers_);
    status = p_proxy_handler->add_client(client_fd);
    //
    // If the stream does not provide JIT connection, connect right now.
    if(status == socketlib::COMPLETE &&
       p_proxy_handler->p_request_stream_handler_->get_forward_address_status() ==
       i_proxy_stream_handler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS){
      
      p_proxy_handler->initiate_server_connect(p_proxy_handler->server_addr_);      
    }

    //
    // Must assume the client is ready to read after it connects.
#warning should we attempt to read?    
//    p_proxy_handler->handle_event(client_fd, EV_READ);
  }
  p_poller_->add_event(event, EV_READ|EV_WRITE, 0);
  return 0;
}  

int new_connection_handler::accept_client(int proxy_server_socket)
{
  struct sockaddr_in peer_addr;
  socklen_t peer_len = sizeof(peer_addr);  
  return ::accept(proxy_server_socket, reinterpret_cast<sockaddr*>(&peer_addr), &peer_len);
}

} //namespace proxylib

