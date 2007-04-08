//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_REQUEST_BUFFER_WRAPPER_HPP
#define SF_REQUEST_BUFFER_WRAPPER_HPP

#include "message_buffer.hpp"

namespace switchflow{
namespace http{

class request_buffer_wrapper
{
public:
  enum VERSION{
    HTTP1,
    HTTP1_1,
    INVALID
  };
  
  request_buffer_wrapper(message_buffer& message_buffer);
  virtual ~request_buffer_wrapper();

  read_write_buffer& get_method();
  read_write_buffer& get_uri();
  read_write_buffer& get_http_version_buffer();
  VERSION get_http_version();
  read_write_buffer& get_field_value_n(unsigned int n);
  bool get_header_with_name_index(char* header_name, unsigned int& index);
      
private:
  message_buffer& message_buffer_;
};

} //namespace http
} //namespace switchflow

#endif 
