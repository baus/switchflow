//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (c) Christopher Baus.  All Rights Reserved.
#include <fcntl.h>

#include <util/logger.hpp>
#include <event/poller.hpp>
#include <socketlib/connection.hpp>

#include "ProxyHandler.h"

namespace proxylib{


ProxyHandler::ProxyHandler(eventlib::poller* pPoller,
                           unsigned int bufferLength,
                           unsigned int clientTimeoutMilliseconds, 
                           unsigned int serverTimeoutMilliseconds,
                           const sockaddr& serverAddr,
                           boost::function<i_pipeline_data* ()> pipeline_data_factory):
  pPoller_(pPoller),
  pProxyHandlers_(0),
  clientData_(bufferLength),
  serverData_(bufferLength),
  pRequestStreamHandlers_(0),
  pResponseStreamHandlers_(0),
  pRequestStreamHandler_(0),
  pResponseStreamHandler_(0),
  clientTimeoutMilliseconds_(clientTimeoutMilliseconds),
  serverTimeoutMilliseconds_(serverTimeoutMilliseconds),
  pipeline_data_queue_(pipeline_data_factory),
  serverAddr_(serverAddr)
{
}

//
// copy construction
//
// Copy contruction is used by the memory management system.
ProxyHandler::ProxyHandler(const ProxyHandler& rhs):
  pProxyHandlers_(0),
  clientData_(rhs.clientData_),
  serverData_(rhs.serverData_),
  pRequestStreamHandlers_(0),
  pResponseStreamHandlers_(0),
  pRequestStreamHandler_(0),
  pResponseStreamHandler_(0),
  clientTimeoutMilliseconds_(rhs.clientTimeoutMilliseconds_),
  serverTimeoutMilliseconds_(rhs.serverTimeoutMilliseconds_),
  pipeline_data_queue_(rhs.pipeline_data_queue_),
  serverAddr_(rhs.serverAddr_)
{
}


ProxyHandler::~ProxyHandler()
{
  shutdown();
}

void ProxyHandler::reset(PessimisticMemoryManager<ProxyHandler>*        pProxyHandlers,
                         PessimisticMemoryManager<IProxyStreamHandler>* pRequestStreamHandlers,
                         PessimisticMemoryManager<IProxyStreamHandler>* pResponseStreamHandlers)
{
  serverData_.reset();
  clientData_.reset();
  pProxyHandlers_          = pProxyHandlers;
  pRequestStreamHandlers_  = pRequestStreamHandlers;
  pResponseStreamHandlers_ = pResponseStreamHandlers;
  pRequestStreamHandler_   = pRequestStreamHandlers_->allocateElement();
  requestStream_.reset(this, &clientData_, &serverData_);
  pRequestStreamHandler_->reset(&requestStream_);
  pResponseStreamHandler_  = pResponseStreamHandlers_->allocateElement();
  responseStream_.reset(this, &serverData_, &clientData_);
  pResponseStreamHandler_->reset(&responseStream_);
  pipeline_data_queue_.empty_queue();
}

int ProxyHandler::handle_event(int fd, short revents, eventlib::event& event)
{
  bool bRequestLineComplete  = false;
  bool bResponseLineComplete = false;
  if(revents & EV_WRITE){
    handlePollout(fd);
  }
  if(revents & EV_READ){
    handlePollin(fd);
  }

  socketlib::STATUS status;

  status = checkForServerConnect(fd);

  if(status != socketlib::COMPLETE && status != socketlib::INCOMPLETE){
    log_info("failed connecting to server");
    shutdown();
    return -1;
  }

  status = handleStream(serverData_, pResponseStreamHandler_);
  if(status == socketlib::COMPLETE){
    bResponseLineComplete = true;
  }
  else if(status == socketlib::DENY){
    shutdown();
    return -1;
  }
  else{
    status = handleStream(clientData_, pRequestStreamHandler_);    
    if(status == socketlib::DENY){
      shutdown_request();
      //
      // Give the response a chance to send an internal error
      status = handleStream(serverData_, pResponseStreamHandler_);
      if(status == socketlib::COMPLETE){
        bResponseLineComplete = true;
      }
      else if(status == socketlib::DENY){
        shutdown();
        return -1;
      }
    }
  }

  if(bResponseLineComplete){
    shutdown();
    return 0;
  }

  int clientEv = 0;
  if(!clientData_.ready_to_read() && clientData_.open_for_read()){
    clientEv |= EV_READ;
  }
  if(!clientData_.ready_to_write() && clientData_.open_for_write()){
    clientEv |= EV_WRITE;
  }

  int serverEv = 0;
  if(!serverData_.ready_to_read() && serverData_.open_for_read()){
    serverEv |= EV_READ;
  }
  if(!serverData_.ready_to_write() && serverData_.open_for_write()){
    serverEv |= EV_WRITE;
  }
  //
  // If a read or write event isn't registered, the 
  // connection will dead lock if the server is connected.
  assert(clientEv || serverEv || 
         !(serverData_.open_for_read() || serverData_.open_for_write()) );
  if(clientEv){
    //
    // event will be deleted if it is already pending.
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    pPoller_->add_event(clientEvent_, clientEv, 0);
  }
  if(serverEv){
    //
    // event will be deleted if it is already pending
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    pPoller_->add_event(serverEvent_, serverEv, 0);
  }
}

void ProxyHandler::shutdown()
{
  if(clientData_.fd() != -1){
    pPoller_->del_event(clientEvent_);
    ::close(clientData_.fd());
  }
  if(serverData_.fd() != -1){
    pPoller_->del_event(serverEvent_);
    ::close(serverData_.fd());
  }

  if(pRequestStreamHandler_ != 0 || pResponseStreamHandler_ != 0){
    pProxyHandlers_->releaseElement(this);
  }
  if(pRequestStreamHandler_ != 0){ 
    pRequestStreamHandlers_->releaseElement(pRequestStreamHandler_); 
  }
  if(pResponseStreamHandler_ != 0){
    pResponseStreamHandlers_->releaseElement(pResponseStreamHandler_);
  }
  pRequestStreamHandler_  = 0;
  pResponseStreamHandler_ = 0;
}

void ProxyHandler::shutdown_request()
{
  ::shutdown(clientData_.fd(), SHUT_RD);
}

socketlib::STATUS ProxyHandler::handleStream(socketlib::connection& srcData,
                                             IProxyStreamHandler* pStreamHandler)
{
  socketlib::STATUS status;
  for(;;){
      //
    // Note: If the client has already disconnected, we should
    // come back through here after the server connects, then
    // drop the server connection.  That's pretty clean.
    if(srcData.state() != socketlib::connection::NOT_CONNECTED &&
       srcData.state() != socketlib::connection::CONNECTING){
      status = srcData.non_blocking_read();
    }
    assert(pStreamHandler->GetProxyStreamInterface() != 0);
    status = pStreamHandler->processData(srcData.readBuffer());
    
    socketlib::STATUS connectStatus;
    connectStatus = attemptJITServerConnect(srcData, pStreamHandler);
    if(connectStatus != socketlib::COMPLETE){
      return connectStatus;
    }

    if(status == socketlib::COMPLETE){
      //
      // The current buffer is completely handled.  It DOES
      // NOT mean that a parse operation is complete.  That is subtly, but
      // importantly different.  For instance if a body crosses two buffers, but
      // the current buffer read has been fully processed and is considered socketlib::COMPLETE here
      // even though we haven't parsed the complete body...  A miss understanding
      // here caused a state jam from the BodyParser, which was fixed.
      if(!srcData.ready_to_read()){
        if(srcData.state() == socketlib::connection::READ_SHUT ||
           srcData.state() == socketlib::connection::HUNGUP){
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
      srcData.state(socketlib::connection::READ_SHUT);
      return socketlib::COMPLETE;
    }
    else if(status == socketlib::DEST_CLOSED){
      //
      // Can't send anymore data.  This 
      // signals that proxy operation is complete.
      if(!srcData.readBuffer().fullyWritten()){
        log_info("destination disconnected before all data was transfered.");
      }
      return socketlib::COMPLETE;  
    }
    else if(status == socketlib::DENY){
      return socketlib::DENY;
    }
  }
}




socketlib::STATUS ProxyHandler::attemptJITServerConnect(socketlib::connection& src, IProxyStreamHandler* pStreamHandler)
{
  //
  // check if we should connect to the server.
  if(&src == &clientData_){
    if(serverData_.state() == socketlib::connection::NOT_CONNECTED){
      //
      // If StreamHandler doesn't provide the forward address, then
      // the logic is screwed up somewhere.
      IProxyStreamHandler::FORWARD_ADDRESS_STATUS addressStatus =
        pStreamHandler->getForwardAddressStatus();
      
      assert(addressStatus != IProxyStreamHandler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS);
      
      if(IProxyStreamHandler::ERROR_DETERMINING_FORWARD_ADDRESS == addressStatus){
        return socketlib::DENY;
      }
      sockaddr serverAddr;
      bool connect = false;
      if(IProxyStreamHandler::FORWARD_ADDRESS_AVAILABLE == addressStatus){
        serverAddr = pStreamHandler->getForwardAddressFromStream();
        connect = true;
      }
      else if(IProxyStreamHandler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS == addressStatus){
        serverAddr = serverAddr_;
        connect = true;
      }
      if(connect){
        socketlib::STATUS connectStatus = initiateServerConnect(serverAddr);
        if(socketlib::COMPLETE != connectStatus){
          return connectStatus;
        }
      }
    }
  }
  return socketlib::COMPLETE;
}

socketlib::STATUS ProxyHandler::handleServerConnect()
{
  //
  // Looks like a server connect.  Let's see what happened.
  int error;
  socklen_t sizeofError = sizeof(error);
  int optErr = getsockopt(serverData_.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeofError);
  if(error == 0 && optErr == 0){
    serverData_.state(socketlib::connection::CONNECTED);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS ProxyHandler::addClient(int clientSocket)
{
  assert(clientData_.fd() == -1);
  clientData_.fd(clientSocket);
  clientData_.state(socketlib::connection::CONNECTED);
  clientEvent_.set(clientSocket, this);
  //
  // It would be tempting to schedule a timeout here, but that will
  // happen as soon as we attempt any I/O the socket that would block.  Remember we
  // must assume the socket is ready to read and/or write
  pPoller_->add_event(clientEvent_, EV_READ|EV_WRITE, 0);
  
  return socketlib::COMPLETE;
}

socketlib::STATUS ProxyHandler::initiateServerConnect(const sockaddr& serverAddr)
{
  // 
  // create a socket
  serverData_.fd(::socket(AF_INET, SOCK_STREAM, 0));
  
  if (serverData_.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    //
    // Need to go into cleanup mode here.
    return socketlib::CONNECTION_FAILED;
  }

  //
  // add to the poller.
  serverEvent_.set(serverData_.fd(), this);
  pPoller_->add_event(serverEvent_, EV_READ|EV_WRITE, 0);
    
  if (::connect(serverData_.fd(), 
                &serverAddr, 
                sizeof(serverAddr)) == -1){
    if(errno != EINPROGRESS){
      //
      //
      log_info("failed connecting to forward server");
      //
      // need clean shutdown
      return socketlib::CONNECTION_FAILED;
    }
    serverData_.state(socketlib::connection::CONNECTING);
  }
  else{
    //
    // Connect immediately succeeded.  This should never really happen.  Hmm not sure
    // if this is really an error.
    CHECK_CONDITION_VAL(false, "connect returned immediately.  is the fd NON-BLOCKING?", errno);
  }
  return socketlib::COMPLETE;
}

void ProxyHandler::handleTimeout(int fd)
{
  if(serverData_.fd() == fd){
    log_info("server timed out");
    assert(!pPoller_->pending(serverEvent_));
    serverData_.fd(-1);
    serverData_.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else if(clientData_.fd() == fd){
    log_info("client timed out");
    assert(!pPoller_->pending(clientEvent_));
    clientData_.fd(-1);
    clientData_.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else{
    CHECK_CONDITION_VAL(false, "timeout occured on unknown file handle", fd);
  }
  if(serverData_.state() == socketlib::connection::HUNGUP && 
     clientData_.state() == socketlib::connection::HUNGUP){
    shutdown();
  }
}


void ProxyHandler::handlePollin(int fd)
{
  if(fd == clientData_.fd()){
    clientData_.ready_to_read(true);
  }
  else if(fd == serverData_.fd()){
    serverData_.ready_to_read(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd", fd);
  }
}

void ProxyHandler::handlePollout(int fd)
{
  if(fd == clientData_.fd()){
    clientData_.ready_to_write(true);
  }
  else if(fd == serverData_.fd()){
    serverData_.ready_to_write(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd.", fd);
  }
}


socketlib::STATUS ProxyHandler::checkForServerConnect(int fd)
{
  if((fd == serverData_.fd()) && (serverData_.state() == socketlib::connection::CONNECTING)){
    //
    // This event signals that connection to the server is complete.
    return handleServerConnect();
  }
  return socketlib::INCOMPLETE;
}

} // namespace proxylib
