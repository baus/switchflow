//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "client_handler.hpp"

#include <util/logger.hpp>
#include <event/poller.hpp>

#include <fcntl.h>
#include <string>
#include <iostream>
#include <memory>

namespace clientlib{


  client_handler::client_handler(eventlib::poller& poller,
                                 unsigned int buffer_length,
                                 unsigned int server_timeout_seconds,
                                 std::auto_ptr<i_client> p_client,
                                 i_fp_monitor* p_fp_monitor):
    poller_(poller),
    server_data_(buffer_length),
    server_timeout_seconds_(server_timeout_seconds),
    p_client_(p_client),
    p_fp_monitor_(p_fp_monitor)
{
  server_data_.reset();

}

client_handler::~client_handler()
{
}



int client_handler::handle_event(int fd, short revents, eventlib::event& event)
{
  if(revents & EV_TIMEOUT){
    handle_timeout(fd);
    return 0;
  }
  if(revents & EV_WRITE){
    handle_pollout(fd);
  }
  if(revents & EV_READ){
    handle_pollin(fd);
  }

  socketlib::STATUS status;

  status = check_for_server_connect(fd);

  if(status != socketlib::COMPLETE && status != socketlib::INCOMPLETE){
    if(p_client_.get()){
      p_client_->connect_failed();
    }
    shutdown();
    return 0;
  }


  int server_ev = 0;

  for(;;){
    server_data_.non_blocking_read();
  
    status = handle_stream();
    if(status == socketlib::COMPLETE){
      shutdown();
      return 0;
    }
    else if(status == socketlib::READ_INCOMPLETE){
      if(server_data_.ready_to_read()){
        continue;
      }
      if(!server_data_.open_for_read()){
        log_info("Read incomplete, but socket closed", server_data_.state());
        shutdown();
        return 0;
      }
      break;
    }
    else if(status == socketlib::WRITE_INCOMPLETE){
      if(!server_data_.open_for_write()){
        log_info("Write incomplete, but socket closed", server_data_.state());
        shutdown();
        return 0;
      }
      if(server_data_.ready_to_write()){
        log_crit("Stream returned WRITE_INCOMPLETE.  The socket is ready to write");
        assert(server_data_.ready_to_write());
      }
      break;
    }
    else if(status == socketlib::DENY){
      shutdown();
      return 0;
    }
    else if(status == socketlib::DEST_CLOSED){
      shutdown();
      return 0;
    }
    else if(status == socketlib::INCOMPLETE){
      log_crit("Not expecting incomplete return value");
      assert(false);
    }
    else{
      CHECK_CONDITION_VAL(false, "unknown return value from handle_stream", status);
    }
  }
  
  if(!server_data_.ready_to_read() && server_data_.open_for_read()){
    server_ev |= EV_READ;
  }
  if(!server_data_.ready_to_write() && server_data_.open_for_write()){
    server_ev |= EV_WRITE;
  }
  if(server_ev){
    poller_.add_event(server_event_, server_ev, server_timeout_seconds_);
  }
  else{
    CHECK_CONDITION_VAL(false, "didn't and event for FD", server_data_.fd());
  }
  return 0;
}

void client_handler::shutdown()
{
  if(server_data_.fd() != -1){
    if(poller_.pending(server_event_)){
      poller_.del_event(server_event_);
    }
    ::close(server_data_.fd());
    if(p_fp_monitor_){
      p_fp_monitor_->remove_fd(server_data_.fd());
    }
    
    server_data_.fd(-1);
  }
  if(p_client_.get()){
    p_client_->shutdown();
  }
  else{
    log_info("shutting down client with no i_client handler");
  }
  delete this;
} 

socketlib::STATUS client_handler::handle_server_connect()
{
  //
  // Looks like a server connect.  Let's see what happened.
  int error;
  socklen_t sizeof_error = sizeof(error);
  int opt_err = getsockopt(server_data_.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeof_error);
  if(error == 0 && opt_err == 0){
    server_data_.state(socketlib::connection::CONNECTED);
    p_client_->connect(server_data_);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS client_handler::initiate_server_connect(const sockaddr_in& server_addr)
{
  //
  // create a socket
  server_data_.fd(::socket(AF_INET, SOCK_STREAM, 0));

  if(p_fp_monitor_){
    p_fp_monitor_->add_fd(server_data_.fd());
  }
  if (server_data_.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    shutdown();
    return socketlib::CONNECTION_FAILED;
  }

  server_data_.set_nonblocking();
  if (::connect(server_data_.fd(),
                (struct sockaddr*)&server_addr,
                sizeof(server_addr)) == -1){
    if(errno != EINPROGRESS){
      //
      //
      log_error("server connect returned unknown value", errno);
      log_error("server fd", server_data_.fd());
      if(p_client_.get()){
        p_client_->connect_failed();
      }
      shutdown();
      return socketlib::CONNECTION_FAILED;
    }
    //
    // add to the poller.
    server_event_.set(server_data_.fd(), this);
    poller_.add_event(server_event_, EV_READ|EV_WRITE, server_timeout_seconds_);
    server_data_.state(socketlib::connection::CONNECTING);
  }
  else{
    //
    // Connect immediately succeeded.  This should never really happen.  Hmm not sure
    // if this is really an error.
    log_error("connect returned immediately, is the fd NON-BLOCKING? errno", errno);
    // this could mean that the address is invalid.

    if(p_client_.get()){
      p_client_->connect_failed();
    }
    shutdown();
    return socketlib::CONNECTION_FAILED;
  }
  return socketlib::COMPLETE;
}

void client_handler::handle_timeout(int fd)
{
  CHECK_CONDITION_VAL(fd == server_data_.fd(), "timeout on unknown fd", fd);
  if(p_client_.get()){
    p_client_->timeout();
  }
  shutdown();
  
}


void client_handler::handle_pollin(int fd)
{
  CHECK_CONDITION_VAL(fd == server_data_.fd(), "notified on unknown fd", fd);
  server_data_.ready_to_read(true);
}

void client_handler::handle_pollout(int fd)
{
  CHECK_CONDITION_VAL(fd == server_data_.fd(), "notified on unknown fd", fd);
  server_data_.ready_to_write(true);
  
}


socketlib::STATUS client_handler::check_for_server_connect(int fd)
{
  CHECK_CONDITION_VAL(fd == server_data_.fd(), "invalid fd to connect", fd);
  if(server_data_.state() == socketlib::connection::CONNECTING){
    //
    // This event signals that connection to the server is complete.
    return handle_server_connect();
  }
  return socketlib::INCOMPLETE;
}

client_handler* client_handler::clone()
{
  return NULL;
}


//
// Should break this into handle request and response
// 
socketlib::STATUS client_handler::handle_stream()
{
  if(p_client_.get()){
    return p_client_->handle_stream(server_data_);
  }
  return socketlib::DENY;
}


i_client* client_handler::get_client()
{
  return p_client_.get();
}

socketlib::connection* client_handler::get_socket_data()
{
  return &server_data_; 
}

void client_handler::dns_failed()
{
  if(p_client_.get()){
    p_client_->dns_failed();
  }
}


}
// namespace
