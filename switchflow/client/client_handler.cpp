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
    m_serverData(bufferLength),
    m_serverTimeoutSeconds(serverTimeoutSeconds),
    p_client_(p_client),
    p_fp_monitor_(p_fp_monitor)
{
  m_serverData.reset();

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
    m_serverData.non_blocking_read();
  
    status = handleStream();
    if(status == socketlib::COMPLETE){
      shutdown();
      return 0;
    }
    else if(status == socketlib::READ_INCOMPLETE){
      if(m_serverData.ready_to_read()){
        continue;
      }
      if(!m_serverData.open_for_read()){
        log_info("Read incomplete, but socket closed", m_serverData.state());
        shutdown();
        return 0;
      }
      break;
    }
    else if(status == socketlib::WRITE_INCOMPLETE){
      if(!m_serverData.open_for_write()){
        log_info("Write incomplete, but socket closed", m_serverData.state());
        shutdown();
        return 0;
      }
      if(m_serverData.ready_to_write()){
        log_crit("Stream returned WRITE_INCOMPLETE.  The socket is ready to write");
        assert(m_serverData.ready_to_write());
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
  
  if(!m_serverData.ready_to_read() && m_serverData.open_for_read()){
    serverEv |= EV_READ;
  }
  if(!m_serverData.ready_to_write() && m_serverData.open_for_write()){
    serverEv |= EV_WRITE;
  }
  if(serverEv){
    poller_.add_event(m_serverEvent, serverEv, m_serverTimeoutSeconds);
  }
  else{
    CHECK_CONDITION_VAL(false, "didn't and event for FD", m_serverData.fd());
  }
  return 0;
}

void client_handler::shutdown()
{
  if(m_serverData.fd() != -1){
    if(poller_.pending(m_serverEvent)){
      poller_.del_event(m_serverEvent);
    }
    ::close(m_serverData.fd());
    if(p_fp_monitor_){
      p_fp_monitor_->remove_fd(m_serverData.fd());
    }
    
    m_serverData.fd(-1);
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
  int optErr = getsockopt(m_serverData.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeofError);
  if(error == 0 && optErr == 0){
    m_serverData.state(socketlib::connection::CONNECTED);
    p_client_->connect(m_serverData);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS client_handler::initiate_server_connect(const sockaddr_in& serverAddr)
{
  //
  // create a socket
  m_serverData.fd(::socket(AF_INET, SOCK_STREAM, 0));

  if(p_fp_monitor_){
    p_fp_monitor_->add_fd(m_serverData.fd());
  }
  if (m_serverData.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    shutdown();
    return socketlib::CONNECTION_FAILED;
  }

  m_serverData.set_nonblocking();
  if (::connect(m_serverData.fd(),
                (struct sockaddr*)&serverAddr,
                sizeof(serverAddr)) == -1){
    if(errno != EINPROGRESS){
      //
      //
      log_error("server connect returned unknown value", errno);
      log_error("server fd", m_serverData.fd());
      if(p_client_.get()){
        p_client_->connect_failed();
      }
      shutdown();
      return socketlib::CONNECTION_FAILED;
    }
    //
    // add to the poller.
    m_serverEvent.set(m_serverData.fd(), this);
    poller_.add_event(m_serverEvent, EV_READ|EV_WRITE, m_serverTimeoutSeconds);
    m_serverData.state(socketlib::connection::CONNECTING);
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
  CHECK_CONDITION_VAL(fd == m_serverData.fd(), "timeout on unknown fd", fd);
  if(p_client_.get()){
    p_client_->timeout();
  }
  shutdown();
  
}


void client_handler::handlePollin(int fd)
{
  CHECK_CONDITION_VAL(fd == m_serverData.fd(), "notified on unknown fd", fd);
  m_serverData.ready_to_read(true);
}

void client_handler::handlePollout(int fd)
{
  CHECK_CONDITION_VAL(fd == m_serverData.fd(), "notified on unknown fd", fd);
  m_serverData.ready_to_write(true);
  
}


socketlib::STATUS client_handler::checkForServerConnect(int fd)
{
  CHECK_CONDITION_VAL(fd == m_serverData.fd(), "invalid fd to connect", fd);
  if(m_serverData.state() == socketlib::connection::CONNECTING){
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
    return p_client_->handle_stream(m_serverData);
  }
  return socketlib::DENY;
}


i_client* client_handler::get_client()
{
  return p_client_.get();
}

socketlib::connection* client_handler::get_socket_data()
{
  return &m_serverData; 
}

void client_handler::dns_failed()
{
  if(p_client_.get()){
    p_client_->dns_failed();
  }
}


}
// namespace
