//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <fcntl.h>

#include <util/logger.hpp>
#include <event/poller.hpp>
#include <socketlib/connection.hpp>

#include "proxy_handler.h"

namespace proxylib{


proxy_handler::proxy_handler(eventlib::poller* p_poller,
                           unsigned int buffer_length,
                           unsigned int client_timeout_milliseconds, 
                           unsigned int server_timeout_milliseconds,
                           const sockaddr& server_addr,
                           boost::function<i_pipeline_data* ()> pipeline_data_factory):
  p_poller_(p_poller),
  p_proxy_handlers_(0),
  client_data_(buffer_length),
  server_data_(buffer_length),
  p_request_stream_handlers_(0),
  p_response_stream_handlers_(0),
  p_request_stream_handler_(0),
  p_response_stream_handler_(0),
  client_timeout_milliseconds_(client_timeout_milliseconds),
  server_timeout_milliseconds_(server_timeout_milliseconds),
  pipeline_data_queue_(pipeline_data_factory),
  server_addr_(server_addr)
{
}

//
// copy construction
//
// Copy contruction is used by the memory management system.
proxy_handler::proxy_handler(const proxy_handler& rhs):
  p_proxy_handlers_(0),
  client_data_(rhs.client_data_),
  server_data_(rhs.server_data_),
  p_request_stream_handlers_(0),
  p_response_stream_handlers_(0),
  p_request_stream_handler_(0),
  p_response_stream_handler_(0),
  client_timeout_milliseconds_(rhs.client_timeout_milliseconds_),
  server_timeout_milliseconds_(rhs.server_timeout_milliseconds_),
  pipeline_data_queue_(rhs.pipeline_data_queue_),
  server_addr_(rhs.server_addr_)
{
}


proxy_handler::~proxy_handler()
{
  shutdown();
}

void proxy_handler::reset(pessimistic_memory_manager<proxy_handler>*        p_proxy_handlers,
                         pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handlers,
                         pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handlers)
{
  server_data_.reset();
  client_data_.reset();
  p_proxy_handlers_          = p_proxy_handlers;
  p_request_stream_handlers_  = p_request_stream_handlers;
  p_response_stream_handlers_ = p_response_stream_handlers;
  p_request_stream_handler_   = p_request_stream_handlers_->allocate_element();
  request_stream_.reset(this, &client_data_, &server_data_);
  p_request_stream_handler_->reset(&request_stream_);
  p_response_stream_handler_  = p_response_stream_handlers_->allocate_element();
  response_stream_.reset(this, &server_data_, &client_data_);
  p_response_stream_handler_->reset(&response_stream_);
  pipeline_data_queue_.empty_queue();
}

int proxy_handler::handle_event(int fd, short revents, eventlib::event& event)
{
  bool b_request_line_complete  = false;
  bool b_response_line_complete = false;
  if(revents & EV_WRITE){
    handle_pollout(fd);
  }
  if(revents & EV_READ){
    handle_pollin(fd);
  }

  socketlib::STATUS status;

  status = check_for_server_connect(fd);

  if(status != socketlib::COMPLETE && status != socketlib::INCOMPLETE){
    log_info("failed connecting to server");
    shutdown();
    return -1;
  }

  status = handle_stream(server_data_, p_response_stream_handler_);
  if(status == socketlib::COMPLETE){
    b_response_line_complete = true;
  }
  else if(status == socketlib::DENY){
    shutdown();
    return -1;
  }
  else{
    status = handle_stream(client_data_, p_request_stream_handler_);    
    if(status == socketlib::DENY){
      shutdown_request();
      //
      // Give the response a chance to send an internal error
      status = handle_stream(server_data_, p_response_stream_handler_);
      if(status == socketlib::COMPLETE){
        b_response_line_complete = true;
      }
      else if(status == socketlib::DENY){
        shutdown();
        return -1;
      }
    }
  }

  if(b_response_line_complete){
    shutdown();
    return 0;
  }

  int client_ev = 0;
  if(!client_data_.ready_to_read() && client_data_.open_for_read()){
    client_ev |= EV_READ;
  }
  if(!client_data_.ready_to_write() && client_data_.open_for_write()){
    client_ev |= EV_WRITE;
  }

  int server_ev = 0;
  if(!server_data_.ready_to_read() && server_data_.open_for_read()){
    server_ev |= EV_READ;
  }
  if(!server_data_.ready_to_write() && server_data_.open_for_write()){
    server_ev |= EV_WRITE;
  }
  //
  // If a read or write event isn't registered, the 
  // connection will dead lock if the server is connected.
  assert(client_ev || server_ev || 
         !(server_data_.open_for_read() || server_data_.open_for_write()) );
  if(client_ev){
    //
    // event will be deleted if it is already pending.
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    p_poller_->add_event(client_event_, client_ev, 0);
  }
  if(server_ev){
    //
    // event will be deleted if it is already pending
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    p_poller_->add_event(server_event_, server_ev, 0);
  }
}

void proxy_handler::shutdown()
{
  if(client_data_.fd() != -1){
    p_poller_->del_event(client_event_);
    ::close(client_data_.fd());
  }
  if(server_data_.fd() != -1){
    p_poller_->del_event(server_event_);
    ::close(server_data_.fd());
  }

  if(p_request_stream_handler_ != 0 || p_response_stream_handler_ != 0){
    p_proxy_handlers_->release_element(this);
  }
  if(p_request_stream_handler_ != 0){ 
    p_request_stream_handlers_->release_element(p_request_stream_handler_); 
  }
  if(p_response_stream_handler_ != 0){
    p_response_stream_handlers_->release_element(p_response_stream_handler_);
  }
  p_request_stream_handler_  = 0;
  p_response_stream_handler_ = 0;
}

void proxy_handler::shutdown_request()
{
  ::shutdown(client_data_.fd(), SHUT_RD);
}

socketlib::STATUS proxy_handler::handle_stream(socketlib::connection& src_data,
                                             i_proxy_stream_handler* p_stream_handler)
{
  socketlib::STATUS status;
  for(;;){
      //
    // Note: If the client has already disconnected, we should
    // come back through here after the server connects, then
    // drop the server connection.  That's pretty clean.
    if(src_data.state() != socketlib::connection::NOT_CONNECTED &&
       src_data.state() != socketlib::connection::CONNECTING){
      status = src_data.non_blocking_read();
    }
    assert(p_stream_handler->get_proxy_stream_interface() != 0);
    status = p_stream_handler->process_data(src_data.read_buffer());
    
    socketlib::STATUS connect_status;
    connect_status = attempt_jit_server_connect(src_data, p_stream_handler);
    if(connect_status != socketlib::COMPLETE){
      return connect_status;
    }

    if(status == socketlib::COMPLETE){
      //
      // The current buffer is completely handled.  It DOES
      // NOT mean that a parse operation is complete.  That is subtly, but
      // importantly different.  For instance if a body crosses two buffers, but
      // the current buffer read has been fully processed and is considered socketlib::COMPLETE
      // here even though we haven't parsed the complete body...  A misunderstanding
      // here caused a state jam from the body_parser, which was fixed.
      if(!src_data.ready_to_read()){
        if(src_data.state() == socketlib::connection::READ_SHUT ||
           src_data.state() == socketlib::connection::HUNGUP){
          return socketlib::COMPLETE;
        }
        return socketlib::INCOMPLETE;      
      }
      //
      // continue through the loop and read again.
    }
    else if(status == socketlib::INCOMPLETE){
      //
      // The current operation block while  writing.  Might also need to continue processing
      // the current buffer.  Read won't read if the buffer isn't marked as fully
      // written.
      return socketlib::INCOMPLETE;
    }
    else if(status == socketlib::CONNECTION_FAILED){
      //
      // I can't think of a reason why this would happen.
      // Should probably assert here.
      return socketlib::CONNECTION_FAILED;
    }
    else if(status == socketlib::SRC_CLOSED){
      //
      // The handler refuses to proxy any more data (ie done pushing denyed
      // request).
      src_data.state(socketlib::connection::READ_SHUT);
      return socketlib::COMPLETE;
    }
    else if(status == socketlib::DEST_CLOSED){
      //
      // Can't send more data.
      // Signal that proxy operation is complete.
      if(!src_data.read_buffer().fully_written()){
        log_info("destination disconnected before all data was transfered.");
      }
      return socketlib::COMPLETE;  
    }
    else if(status == socketlib::DENY){
      return socketlib::DENY;
    }
  }
}




socketlib::STATUS proxy_handler::attempt_jit_server_connect(socketlib::connection& src, i_proxy_stream_handler* p_stream_handler)
{
  //
  // check if we should connect to the server.
  if(&src == &client_data_){
    if(server_data_.state() == socketlib::connection::NOT_CONNECTED){
      //
      // If stream_handler doesn't provide the forward address, then
      // the logic is screwed up somewhere.
      i_proxy_stream_handler::FORWARD_ADDRESS_STATUS address_status =
        p_stream_handler->get_forward_address_status();
      
      assert(address_status != i_proxy_stream_handler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS);
      
      if(i_proxy_stream_handler::ERROR_DETERMINING_FORWARD_ADDRESS == address_status){
        return socketlib::DENY;
      }
      sockaddr server_addr;
      bool connect = false;
      if(i_proxy_stream_handler::FORWARD_ADDRESS_AVAILABLE == address_status){
        server_addr = p_stream_handler->get_forward_address_from_stream();
        connect = true;
      }
      else if(i_proxy_stream_handler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS == address_status){
        server_addr = server_addr_;
        connect = true;
      }
      if(connect){
        socketlib::STATUS connect_status = initiate_server_connect(server_addr);
        if(socketlib::COMPLETE != connect_status){
          return connect_status;
        }
      }
    }
  }
  return socketlib::COMPLETE;
}

socketlib::STATUS proxy_handler::handle_server_connect()
{
  //
  // Looks like a server connect.  Let's see what happened.
  int error;
  socklen_t sizeof_error = sizeof(error);
  int opt_err = getsockopt(server_data_.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeof_error);
  if(error == 0 && opt_err == 0){
    server_data_.state(socketlib::connection::CONNECTED);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS proxy_handler::add_client(int client_socket)
{
  assert(client_data_.fd() == -1);
  client_data_.fd(client_socket);
  client_data_.state(socketlib::connection::CONNECTED);
  client_event_.set(client_socket, this);
  //
  // It would be tempting to schedule a timeout here, but that will
  // happen as soon as we attempt any I/O the socket that would block.  Remember we
  // must assume the socket is ready to read and/or write
  p_poller_->add_event(client_event_, EV_READ|EV_WRITE, 0);
  
  return socketlib::COMPLETE;
}

socketlib::STATUS proxy_handler::initiate_server_connect(const sockaddr& server_addr)
{
  // 
  // create a socket
  server_data_.fd(::socket(AF_INET, SOCK_STREAM, 0));
  
  if (server_data_.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    //
    // Need to go into cleanup mode here.
    return socketlib::CONNECTION_FAILED;
  }

  //
  // add to the poller.
  server_event_.set(server_data_.fd(), this);
  p_poller_->add_event(server_event_, EV_READ|EV_WRITE, 0);
    
  if (::connect(server_data_.fd(), 
                &server_addr, 
                sizeof(server_addr)) == -1){
    if(errno != EINPROGRESS){
      //
      //
      log_info("failed connecting to forward server");
      //
      // need clean shutdown
      return socketlib::CONNECTION_FAILED;
    }
    server_data_.state(socketlib::connection::CONNECTING);
  }
  else{
    //
    // Connect immediately succeeded.  This should never really happen.  Hmm not sure
    // if this is really an error.
    CHECK_CONDITION_VAL(false, "connect returned immediately.  is the fd NON-BLOCKING?", errno);
  }
  return socketlib::COMPLETE;
}

void proxy_handler::handle_timeout(int fd)
{
  if(server_data_.fd() == fd){
    log_info("server timed out");
    assert(!p_poller_->pending(server_event_));
    server_data_.fd(-1);
    server_data_.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else if(client_data_.fd() == fd){
    log_info("client timed out");
    assert(!p_poller_->pending(client_event_));
    client_data_.fd(-1);
    client_data_.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else{
    CHECK_CONDITION_VAL(false, "timeout occured on unknown file handle", fd);
  }
  if(server_data_.state() == socketlib::connection::HUNGUP && 
     client_data_.state() == socketlib::connection::HUNGUP){
    shutdown();
  }
}


void proxy_handler::handle_pollin(int fd)
{
  if(fd == client_data_.fd()){
    client_data_.ready_to_read(true);
  }
  else if(fd == server_data_.fd()){
    server_data_.ready_to_read(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd", fd);
  }
}

void proxy_handler::handle_pollout(int fd)
{
  if(fd == client_data_.fd()){
    client_data_.ready_to_write(true);
  }
  else if(fd == server_data_.fd()){
    server_data_.ready_to_write(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd.", fd);
  }
}


socketlib::STATUS proxy_handler::check_for_server_connect(int fd)
{
  if((fd == server_data_.fd()) && (server_data_.state() == socketlib::connection::CONNECTING)){
    //
    // This event signals that connection to the server is complete.
    return handle_server_connect();
  }
  return socketlib::INCOMPLETE;
}

} // namespace proxylib
