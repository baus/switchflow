//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HTTP_RESPONSE_BUFFER_WRAPPER_HPP
#define SF_HTTP_RESPONSE_BUFFER_WRAPPER_HPP

#include "message_buffer.hpp"

namespace switchflow{
namespace http{
  
class response_buffer_wrapper
{
public:
  response_buffer_wrapper(message_buffer& message_buffer);
  virtual ~response_buffer_wrapper();

  read_write_buffer& get_http_version();
  read_write_buffer& get_status_code();
  read_write_buffer& get_reason_phrase();
  
private:
  message_buffer& message_buffer_;
};

} //namespace http
} //namespace switchflow

#endif
