//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

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
#include <util/pessimistic_memory_manager.hpp>
#include <util/read_write_buffer.hpp>
#include <event/event.hpp>
#include <event/i_event_handler.hpp>

#include "proxy_stream_interface.hpp"
#include "i_proxy_stream_handler.hpp"
#include "pipeline_data_queue.hpp"



namespace proxylib{

// proxy_handler handles notifications from proxy connections.  It maintains
// the state of the client and server socket handles.  There is one proxy_handler
// instance for a pair of client/server handles.
//
// The proxy_handler is a generic concept, and no application level processing
// happens within.  The proxy_handler just abstracts the socket handling and notificaiton
// for a generic proxy server.  The application level knows nothing of the underlying
// socket mechanism, but must be aware that this a non-blocking, event driven mechanism.  
// When ever the higher application forwards data, it must be prepared to return immediately
// as the forward procedure could block.
//
class proxy_handler: public eventlib::i_event_handler
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
  friend class proxy_stream_interface;
  

  //
  // proxy_handler construction
  //
  // @param buffer_length length of the input buffers for the client and server sockets.
  // @param client_timeout_milliseconds the number of milliseconds to wait between
  // i/o events from the client before assuming the
  // client is dead.
  //
  // @param server_timeout_milliseconds the number of milliseconds to wait between
  // i/o events from the client before assuming the
  // server is dead.
  //
  // @param server_addr The address of the server socket to connect to.
  //
  proxy_handler(eventlib::poller* p_poller,
               unsigned int buffer_length, 
               unsigned int client_timeout_milliseconds, 
               unsigned int server_timeout_milliseconds,
               const sockaddr& server_addr,
               boost::function<i_pipeline_data* ()> pipeline_data_factory);

  //
  // copy construction
  //
  // Copy contruction is used by the memory management system.    
  //
  proxy_handler(const proxy_handler& rhs);
  

  virtual ~proxy_handler();
 

  //
  // This is used to re-initialize the object after it has been
  // recycled from the Pessimistic Memory Manager.
  //
  // @param p_proxy_handlers back pointer to the memory manager from which this
  // instance has been allocated.  Used to free the current instance.
  //
  void reset(pessimistic_memory_manager<proxy_handler>* p_proxy_handlers,
       pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handler,
       pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handler);
  
  /**
   * Overrides the rn_poller::i_event::notify() handler.  This will be 
   * called by the rn_poller framework.
   */     
  virtual int handle_event(int fd, short revents, eventlib::event& event);
  
 private:
  /**
   * Contains information about a socket connection.  It should only be used
   * by proxy_handler.  It is not intended to be used from outside this class.
   */
  friend class new_connection_handler;
   
  socketlib::STATUS add_client(int proxy_server_socket);
  
  void handle_timeout(int fd);
  
  /**
   * closes connections and removes sockets from notification.  This should only be
   * called once all data has been processed and flushed from the input buffers.  
   *
   * @param p_poller This is the poller instance that is passed to notify.
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
  socketlib::STATUS handle_stream(socketlib::connection& src_data,
				  i_proxy_stream_handler* p_stream_handler);  

  void handle_pollin(int fd);

  void handle_pollout(int fd);

  socketlib::STATUS check_for_server_connect(int fd);
  
  
  //
  // This initiates the server connection.  The connection will not
  // be complete until a POLLOUT is received on the server socket handle.
  //
  socketlib::STATUS initiate_server_connect(const sockaddr& server_addr);

  //
  // handles the completion of server socket connection.
  //
  socketlib::STATUS handle_server_connect();

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
  socketlib::STATUS attempt_jit_server_connect(socketlib::connection& src, i_proxy_stream_handler* p_stream_handler);
  
  //
  // Delete elements of the pipeline queue.
  // Needs to be done on reset to prevent memory
  // leak on dropped connections.
  //
  void clear_pipeline_queue();

  //
  // Performs a 1/2 duplex shutdown of the request line.
  //
  void shutdown_request();
  
  //
  // A back pointer to the memory_manager managing this instance.  
  // This allows us to delete ourselves from the memory manager. 
  // 
  pessimistic_memory_manager<proxy_handler>* p_proxy_handlers_;

  //
  // data for the client.
  socketlib::connection client_data_;
 
  //
  // data for the server
  socketlib::connection server_data_;

  eventlib::event client_event_;

  eventlib::event server_event_;

  proxy_stream_interface request_stream_;
  
  proxy_stream_interface response_stream_;

  pessimistic_memory_manager<i_proxy_stream_handler>* p_request_stream_handlers_;
 
  pessimistic_memory_manager<i_proxy_stream_handler>* p_response_stream_handlers_;
  
  i_proxy_stream_handler* p_request_stream_handler_;
  
  i_proxy_stream_handler* p_response_stream_handler_;

  unsigned int client_timeout_milliseconds_;
  
  unsigned int server_timeout_milliseconds_;

  // Address of the downstream server.
  //
  sockaddr server_addr_;
  
  eventlib::poller* p_poller_;

  pipeline_data_queue pipeline_data_queue_;

};


} // namespace proxylib 
#endif // PROXY_HANDLERS_H__
