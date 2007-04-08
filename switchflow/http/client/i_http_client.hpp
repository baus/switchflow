//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SS_I_HTTP_CLIENT_HPP
#define SS_I_HTTP_CLIENT_HPP

#include "http_client_handler.hpp"

class i_http_client
{
public:
  virtual ~i_http_client(){}
  virtual void invalid_header() = 0;
  virtual void invalid_body() = 0;
  virtual void dns_failed() = 0;
  virtual void connect_failed() = 0;
  virtual void timeout() = 0;
  virtual void shutdown() = 0;
  virtual void send_header_failed() = 0;
  virtual void send_body_failed() = 0;

  virtual void headers_complete() = 0;
  virtual switchflow::http::STATUS handle_body(read_write_buffer& body, bool b_complete) = 0;
  virtual void request_complete() = 0;
  virtual switchflow::http::message_buffer& get_request() = 0;
  virtual switchflow::http::message_buffer& get_response() = 0;
};
#endif // 
