//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (c) Christopher Baus.  All Rights Reserved.
#include "client_handler.hpp"

#include <util/logger.hpp>
#include <event/poller.hpp>

#include <fcntl.h>
#include <string>
#include <iostream>
#include <memory>

namespace clientlib{


  client_handler::client_handler(eventlib::poller& poller,
                                 unsigned int bufferLength,
                                 unsigned int serverTimeoutSeconds,
                                 std::auto_ptr<i_client> p_client,
                                 i_fp_monitor* p_fp_monitor):
    poller_(poller),
    serverData_(bufferLength),
    serverTimeoutSeconds_(serverTimeoutSeconds),
    p_client_(p_client),
    p_fp_monitor_(p_fp_monitor)
{
  serverData_.reset();

}

client_handler::~client_handler()
{
}



int client_handler::handle_event(int fd, short revents, eventlib::event& event)
{
  if(revents & EV_TIMEOUT){
    handleTimeout(fd);
    return 0;
  }
  if(revents & EV_WRITE){
    handlePollout(fd);
  }
  if(revents & EV_READ){
    handlePollin(fd);
  }

  socketlib::STATUS status;

  status = checkForServerConnect(fd);

  if(status != socketlib::COMPLETE && status != socketlib::INCOMPLETE){
    if(p_client_.get()){
      p_client_->connect_failed();
    }
    shutdown();
    return 0;
  }


  int serverEv = 0;

  for(;;){
    serverData_.non_blocking_read();
  
    status = handleStream();
    if(status == socketlib::COMPLETE){
      shutdown();
      return 0;
    }
    else if(status == socketlib::READ_INCOMPLETE){
      if(serverData_.ready_to_read()){
        continue;
      }
      if(!serverData_.open_for_read()){
        log_info("Read incomplete, but socket closed", serverData_.state());
        shutdown();
        return 0;
      }
      break;
    }
    else if(status == socketlib::WRITE_INCOMPLETE){
      if(!serverData_.open_for_write()){
        log_info("Write incomplete, but socket closed", serverData_.state());
        shutdown();
        return 0;
      }
      if(serverData_.ready_to_write()){
        log_crit("Stream returned WRITE_INCOMPLETE.  The socket is ready to write");
        assert(serverData_.ready_to_write());
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
      CHECK_CONDITION_VAL(false, "unknown return value from handleStream", status);
    }
  }
  
  if(!serverData_.ready_to_read() && serverData_.open_for_read()){
    serverEv |= EV_READ;
  }
  if(!serverData_.ready_to_write() && serverData_.open_for_write()){
    serverEv |= EV_WRITE;
  }
  if(serverEv){
    poller_.add_event(serverEvent_, serverEv, serverTimeoutSeconds_);
  }
  else{
    CHECK_CONDITION_VAL(false, "didn't and event for FD", serverData_.fd());
  }
  return 0;
}

void client_handler::shutdown()
{
  if(serverData_.fd() != -1){
    if(poller_.pending(serverEvent_)){
      poller_.del_event(serverEvent_);
    }
    ::close(serverData_.fd());
    if(p_fp_monitor_){
      p_fp_monitor_->remove_fd(serverData_.fd());
    }
    
    serverData_.fd(-1);
  }
  if(p_client_.get()){
    p_client_->shutdown();
  }
  else{
    log_info("shutting down client with no i_client handler");
  }
  delete this;
} 

socketlib::STATUS client_handler::handleServerConnect()
{
  //
  // Looks like a server connect.  Let's see what happened.
  int error;
  socklen_t sizeofError = sizeof(error);
  int optErr = getsockopt(serverData_.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeofError);
  if(error == 0 && optErr == 0){
    serverData_.state(socketlib::connection::CONNECTED);
    p_client_->connect(serverData_);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS client_handler::initiate_server_connect(const sockaddr_in& serverAddr)
{
  //
  // create a socket
  serverData_.fd(::socket(AF_INET, SOCK_STREAM, 0));

  if(p_fp_monitor_){
    p_fp_monitor_->add_fd(serverData_.fd());
  }
  if (serverData_.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    shutdown();
    return socketlib::CONNECTION_FAILED;
  }

  serverData_.set_nonblocking();
  if (::connect(serverData_.fd(),
                (struct sockaddr*)&serverAddr,
                sizeof(serverAddr)) == -1){
    if(errno != EINPROGRESS){
      //
      //
      log_error("server connect returned unknown value", errno);
      log_error("server fd", serverData_.fd());
      if(p_client_.get()){
        p_client_->connect_failed();
      }
      shutdown();
      return socketlib::CONNECTION_FAILED;
    }
    //
    // add to the poller.
    serverEvent_.set(serverData_.fd(), this);
    poller_.add_event(serverEvent_, EV_READ|EV_WRITE, serverTimeoutSeconds_);
    serverData_.state(socketlib::connection::CONNECTING);
  }
  else{
    int error = errno;
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

void client_handler::handleTimeout(int fd)
{
  CHECK_CONDITION_VAL(fd == serverData_.fd(), "timeout on unknown fd", fd);
  if(p_client_.get()){
    p_client_->timeout();
  }
  shutdown();
  
}


void client_handler::handlePollin(int fd)
{
  CHECK_CONDITION_VAL(fd == serverData_.fd(), "notified on unknown fd", fd);
  serverData_.ready_to_read(true);
}

void client_handler::handlePollout(int fd)
{
  CHECK_CONDITION_VAL(fd == serverData_.fd(), "notified on unknown fd", fd);
  serverData_.ready_to_write(true);
  
}


socketlib::STATUS client_handler::checkForServerConnect(int fd)
{
  CHECK_CONDITION_VAL(fd == serverData_.fd(), "invalid fd to connect", fd);
  if(serverData_.state() == socketlib::connection::CONNECTING){
    //
    // This event signals that connection to the server is complete.
    return handleServerConnect();
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
socketlib::STATUS client_handler::handleStream()
{
  if(p_client_.get()){
    return p_client_->handle_stream(serverData_);
  }
  return socketlib::DENY;
}


i_client* client_handler::get_client()
{
  return p_client_.get();
}

socketlib::connection* client_handler::get_socket_data()
{
  return &serverData_; 
}

void client_handler::dns_failed()
{
  if(p_client_.get()){
    p_client_->dns_failed();
  }
}


}
// namespace
