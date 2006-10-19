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
  m_pPoller(pPoller),
  m_pProxyHandlers(0),
  m_clientData(bufferLength),
  m_serverData(bufferLength),
  m_pRequestStreamHandlers(0),
  m_pResponseStreamHandlers(0),
  m_pRequestStreamHandler(0),
  m_pResponseStreamHandler(0),
  m_clientTimeoutMilliseconds(clientTimeoutMilliseconds),
  m_serverTimeoutMilliseconds(serverTimeoutMilliseconds),
  pipeline_data_queue_(pipeline_data_factory),
  m_serverAddr(serverAddr)
{
}

//
// copy construction
//
// Copy contruction is used by the memory management system.
ProxyHandler::ProxyHandler(const ProxyHandler& rhs):
  m_pProxyHandlers(0),
  m_clientData(rhs.m_clientData),
  m_serverData(rhs.m_serverData),
  m_pRequestStreamHandlers(0),
  m_pResponseStreamHandlers(0),
  m_pRequestStreamHandler(0),
  m_pResponseStreamHandler(0),
  m_clientTimeoutMilliseconds(rhs.m_clientTimeoutMilliseconds),
  m_serverTimeoutMilliseconds(rhs.m_serverTimeoutMilliseconds),
  pipeline_data_queue_(rhs.pipeline_data_queue_),
  m_serverAddr(rhs.m_serverAddr)
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
  m_serverData.reset();
  m_clientData.reset();
  m_pProxyHandlers          = pProxyHandlers;
  m_pRequestStreamHandlers  = pRequestStreamHandlers;
  m_pResponseStreamHandlers = pResponseStreamHandlers;
  m_pRequestStreamHandler   = m_pRequestStreamHandlers->allocateElement();
  m_requestStream.reset(this, &m_clientData, &m_serverData);
  m_pRequestStreamHandler->reset(&m_requestStream);
  m_pResponseStreamHandler  = m_pResponseStreamHandlers->allocateElement();
  m_responseStream.reset(this, &m_serverData, &m_clientData);
  m_pResponseStreamHandler->reset(&m_responseStream);
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

  status = handleStream(m_serverData, m_pResponseStreamHandler);
  if(status == socketlib::COMPLETE){
    bResponseLineComplete = true;
  }
  else if(status == socketlib::DENY){
    shutdown();
    return -1;
  }
  else{
    status = handleStream(m_clientData, m_pRequestStreamHandler);    
    if(status == socketlib::DENY){
      shutdown_request();
      //
      // Give the response a chance to send an internal error
      status = handleStream(m_serverData, m_pResponseStreamHandler);
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
  if(!m_clientData.ready_to_read() && m_clientData.open_for_read()){
    clientEv |= EV_READ;
  }
  if(!m_clientData.ready_to_write() && m_clientData.open_for_write()){
    clientEv |= EV_WRITE;
  }

  int serverEv = 0;
  if(!m_serverData.ready_to_read() && m_serverData.open_for_read()){
    serverEv |= EV_READ;
  }
  if(!m_serverData.ready_to_write() && m_serverData.open_for_write()){
    serverEv |= EV_WRITE;
  }
  //
  // If a read or write event isn't registered, the 
  // connection will dead lock if the server is connected.
  assert(clientEv || serverEv || 
         !(m_serverData.open_for_read() || m_serverData.open_for_write()) );
  if(clientEv){
    //
    // event will be deleted if it is already pending.
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    m_pPoller->add_event(m_clientEvent, clientEv, 0);
  }
  if(serverEv){
    //
    // event will be deleted if it is already pending
    //
    // Note this reshedules the timeout even if the event hasn't changed.
    m_pPoller->add_event(m_serverEvent, serverEv, 0);
  }
}

void ProxyHandler::shutdown()
{
  if(m_clientData.fd() != -1){
    m_pPoller->del_event(m_clientEvent);
    ::close(m_clientData.fd());
  }
  if(m_serverData.fd() != -1){
    m_pPoller->del_event(m_serverEvent);
    ::close(m_serverData.fd());
  }

  if(m_pRequestStreamHandler != 0 || m_pResponseStreamHandler != 0){
    m_pProxyHandlers->releaseElement(this);
  }
  if(m_pRequestStreamHandler != 0){ 
    m_pRequestStreamHandlers->releaseElement(m_pRequestStreamHandler); 
  }
  if(m_pResponseStreamHandler != 0){
    m_pResponseStreamHandlers->releaseElement(m_pResponseStreamHandler);
  }
  m_pRequestStreamHandler  = 0;
  m_pResponseStreamHandler = 0;
}

void ProxyHandler::shutdown_request()
{
  ::shutdown(m_clientData.fd(), SHUT_RD);
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
  if(&src == &m_clientData){
    if(m_serverData.state() == socketlib::connection::NOT_CONNECTED){
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
        serverAddr = m_serverAddr;
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
  int optErr = getsockopt(m_serverData.fd(), SOL_SOCKET, SO_ERROR, &error, &sizeofError);
  if(error == 0 && optErr == 0){
    m_serverData.state(socketlib::connection::CONNECTED);
    return socketlib::COMPLETE;
  }
  return socketlib::CONNECTION_FAILED;
}

socketlib::STATUS ProxyHandler::addClient(int clientSocket)
{
  assert(m_clientData.fd() == -1);
  m_clientData.fd(clientSocket);
  m_clientData.state(socketlib::connection::CONNECTED);
  m_clientEvent.set(clientSocket, this);
  //
  // It would be tempting to schedule a timeout here, but that will
  // happen as soon as we attempt any I/O the socket that would block.  Remember we
  // must assume the socket is ready to read and/or write
  m_pPoller->add_event(m_clientEvent, EV_READ|EV_WRITE, 0);
  
  return socketlib::COMPLETE;
}

socketlib::STATUS ProxyHandler::initiateServerConnect(const sockaddr& serverAddr)
{
  // 
  // create a socket
  m_serverData.fd(::socket(AF_INET, SOCK_STREAM, 0));
  
  if (m_serverData.fd() == -1)  {
    log_info("failed creating forward server socket", errno);
    //
    // Need to go into cleanup mode here.
    return socketlib::CONNECTION_FAILED;
  }

  //
  // add to the poller.
  m_serverEvent.set(m_serverData.fd(), this);
  m_pPoller->add_event(m_serverEvent, EV_READ|EV_WRITE, 0);
    
  if (::connect(m_serverData.fd(), 
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
    m_serverData.state(socketlib::connection::CONNECTING);
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
  if(m_serverData.fd() == fd){
    log_info("server timed out");
    assert(!m_pPoller->pending(m_serverEvent));
    m_serverData.fd(-1);
    m_serverData.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else if(m_clientData.fd() == fd){
    log_info("client timed out");
    assert(!m_pPoller->pending(m_clientEvent));
    m_clientData.fd(-1);
    m_clientData.state(socketlib::connection::HUNGUP);
    ::close(fd);
  }
  else{
    CHECK_CONDITION_VAL(false, "timeout occured on unknown file handle", fd);
  }
  if(m_serverData.state() == socketlib::connection::HUNGUP && 
     m_clientData.state() == socketlib::connection::HUNGUP){
    shutdown();
  }
}


void ProxyHandler::handlePollin(int fd)
{
  if(fd == m_clientData.fd()){
    m_clientData.ready_to_read(true);
  }
  else if(fd == m_serverData.fd()){
    m_serverData.ready_to_read(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd", fd);
  }
}

void ProxyHandler::handlePollout(int fd)
{
  if(fd == m_clientData.fd()){
    m_clientData.ready_to_write(true);
  }
  else if(fd == m_serverData.fd()){
    m_serverData.ready_to_write(true);
  }
  else{
    CHECK_CONDITION_VAL(false, "notified on unknown fd.", fd);
  }
}


socketlib::STATUS ProxyHandler::checkForServerConnect(int fd)
{
  if((fd == m_serverData.fd()) && (m_serverData.state() == socketlib::connection::CONNECTING)){
    //
    // This event signals that connection to the server is complete.
    return handleServerConnect();
  }
  return socketlib::INCOMPLETE;
}

} // namespace proxylib
