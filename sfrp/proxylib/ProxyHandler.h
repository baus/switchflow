//
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef PROXY_HANDLERS_H__
#define PROXY_HANDLERS_H__

// system includes
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// c++ includes
#include <list>

// thirdparty includes
#include <boost/noncopyable.hpp>

// library includes
#include <socketlib/connection.hpp>
#include <socketlib/status.hpp>

// local includes
#include <util/PessimisticMemoryManager.h>
#include <util/read_write_buffer.hpp>
#include <event/event.hpp>
#include <event/i_event_handler.hpp>

#include "ProxyStreamInterface.h"
#include "IProxyStreamHandler.h"
#include "pipeline_data_queue.hpp"



namespace proxylib{

// ProxyHandler handles notifications from proxy connections.  It maintains
// the state of the client and server socket handles.  There is one ProxyHandler
// instance for a pair of client/server handles.
//
// The ProxyHandler is a generic concept, and no application level processing
// happens within.  The ProxyHandler just abstracts the socket handling and notificaiton
// for a generic proxy server.  The application level knows nothing of the underlying
// socket mechanism, but must be aware that this a non-blocking, event driven mechanism.  
// When ever the higher application forwards data, it must be prepared to return immediately
// as the forward procedure could block.
//
class ProxyHandler: public eventlib::i_event_handler
{
 public:
  //
  // This is the external interface used by application level mechanisms
  // to interact with the proxy.  The application level can only request
  // to forward a buffer, or request a connection to the server.
  //
  // The application level should only request a connection once it is
  // sufficently certain that the incoming data is valid.  This is to
  // reduce the load on the server in the case of application DoS attacks.
  //
  friend class ProxyStreamInterface;
  

  //
  // ProxyHandler construction
  //
  // @param bufferLength length of the input buffers for the client and server sockets.
  // @param clientTimeoutMilliseconds the number of milliseconds to wait between
  // i/o events from the client before assuming the
  // client is dead.
  //
  // @param serverTimeoutMilliseconds the number of milliseconds to wait between
  // i/o events from the client before assuming the
  // server is dead.
  //
  // @param serverAddr The address of the server socket to connect to.
  //
  ProxyHandler(eventlib::poller* pPoller,
               unsigned int bufferLength, 
               unsigned int clientTimeoutMilliseconds, 
               unsigned int serverTimeoutMilliseconds,
               const sockaddr& serverAddr,
               boost::function<i_pipeline_data* ()> pipeline_data_factory);

  //
  // copy construction
  //
  // Copy contruction is used by the memory management system.    
  //
  ProxyHandler(const ProxyHandler& rhs);
  

  virtual ~ProxyHandler();
 

  //
  // This is used to re-initialize the object after it has been
  // recycled from the Pessimistic Memory Manager.
  //
  // @param pProxyHandlers back pointer to the memory manager from which this
  // instance has been allocated.  Used to free the
  // current instance.
  //
  void reset(PessimisticMemoryManager<ProxyHandler>* pProxyHandlers,
       PessimisticMemoryManager<IProxyStreamHandler>* pRequestStreamHandler,
       PessimisticMemoryManager<IProxyStreamHandler>* pResponseStreamHandler);
  
  /**
   * Overrides the rnPoller::IEvent::notify() handler.  This will be 
   * called by the rnPoller framework.
   */     
  virtual int handle_event(int fd, short revents, eventlib::event& event);
  
 private:
  /**
   * Contains information about a socket connection.  It should only be used
   * by ProxyHandler.  It is not intended to be used from outside this class.
   */
  friend class NewConnectionHandler;
   
  socketlib::STATUS addClient(int proxyServerSocket);
  
  void handleTimeout(int fd);
  
  /**
   * closes connections and removes sockets from notification.  This should only be
   * called once all data has been processed and flushed from the input buffers.  
   *
   * @param pPoller This is the poller instance that is passed to notify.
   *                it is needed to remove the sockets from notification.
   */
  void shutdown();

  /**
   * Handles transfering data from the client to the server.  It returns when
   * processing becomes I/O bound.  This can happen if the client is not ready
   * to read or the server is not ready to write.  In either case control should
   * continue back to the poll loop.  
   *
   * It also handles setting up the connection to the server.
   */
  socketlib::STATUS ProxyHandler::handleStream(socketlib::connection& srcData,
                                    IProxyStreamHandler* pStreamHandler);  

  void handlePollin(int fd);

  void handlePollout(int fd);

  socketlib::STATUS checkForServerConnect(int fd);
  
  
  //
  // This initiates the server connection.  The connection will not
  // be complete until a POLLOUT is received on the server socket handle.
  //
  socketlib::STATUS initiateServerConnect(const sockaddr& serverAddr);

  //
  // handles the completion of server socket connection.
  //
  socketlib::STATUS handleServerConnect();

  //
  // Check if we need to connect to the downstream server,
  // and initiate connection if required.
  //
  // @param src the source socket. Used to determine if we are
  //            processing the request line of the proxy.
  //
  // @return COMPLETE if connection initiation is successful or if no
  //         connection is required
  //
  socketlib::STATUS attemptJITServerConnect(socketlib::connection& src, IProxyStreamHandler* pStreamHandler);
  
  //
  // Delete elements of the pipeline queue.
  // Needs to be done on reset to prevent memory
  // leak on dropped connections.
  //
  void clearPipelineQueue();

  //
  // Performs a 1/2 duplex shutdown of the request line.
  //
  void shutdown_request();
  
  //
  // A back pointer to the memoryManager managing this instance.  
  // This allows us to delete ourselves from the memory manager. 
  // 
  PessimisticMemoryManager<ProxyHandler>* m_pProxyHandlers;

  //
  // data for the client.
  socketlib::connection m_clientData;
 
  //
  // data for the server
  socketlib::connection m_serverData;

  eventlib::event m_clientEvent;

  eventlib::event m_serverEvent;

  ProxyStreamInterface m_requestStream;
  
  ProxyStreamInterface m_responseStream;

  PessimisticMemoryManager<IProxyStreamHandler>* m_pRequestStreamHandlers;
 
  PessimisticMemoryManager<IProxyStreamHandler>* m_pResponseStreamHandlers;
  
  IProxyStreamHandler* m_pRequestStreamHandler;
  
  IProxyStreamHandler* m_pResponseStreamHandler;

  unsigned int m_clientTimeoutMilliseconds;
  
  unsigned int m_serverTimeoutMilliseconds;

  // Address of the downstream server.
  //
  sockaddr m_serverAddr;
  
  eventlib::poller* m_pPoller;

  pipeline_data_queue pipeline_data_queue_;

};


} // namespace proxylib 
#endif
