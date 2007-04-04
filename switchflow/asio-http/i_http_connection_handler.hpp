//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_I_HTTP_CONNECTION_HPP
#define SF_I_HTTP_CONNECTION_HPP

#include <http/http.hpp>
#include <http/message_buffer.hpp>
#include <asio/buffer.hpp>

class i_http_connection_handler
{
public:
  virtual ~i_http_connection_handler(){}
  virtual void send_failed() = 0;
  virtual void invalid_peer_header() = 0;
  virtual void invalid_peer_body() = 0;
  
  virtual void timeout() = 0;
  virtual void shutdown() = 0;
  
  virtual void headers_complete() = 0;
  virtual void request_complete() = 0;

  virtual void accept_peer_body(asio::mutable_buffer&) = 0;
};

#endif 
