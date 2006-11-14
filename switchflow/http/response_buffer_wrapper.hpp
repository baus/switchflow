//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_HTTP_RESPONSE_BUFFER_WRAPPER_HPP
#define SSD_HTTP_RESPONSE_BUFFER_WRAPPER_HPP

#include "message_buffer.hpp"

namespace http{
  
class http_response_buffer_wrapper
{
public:
  http_response_buffer_wrapper(message_buffer& message_buffer);
  virtual ~http_response_buffer_wrapper();

  read_write_buffer& get_http_version();
  read_write_buffer& get_status_code();
  read_write_buffer& get_reason_phrase();
  
private:
  message_buffer& message_buffer_;
};

}
#endif // SSD_HTTP_RESPONSE_BUFFER_WRAPPER_HP
