//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "request_buffer_wrapper.hpp"

namespace switchflow{
namespace http{

request_buffer_wrapper::request_buffer_wrapper(message_buffer& message_buffer):message_buffer_(message_buffer)
{
}

request_buffer_wrapper::~request_buffer_wrapper()
{
}

read_write_buffer& request_buffer_wrapper::get_method()
{
  return message_buffer_.get_status_line_1();
}

read_write_buffer& request_buffer_wrapper::get_uri()
{
  return message_buffer_.get_status_line_2();
}

read_write_buffer& request_buffer_wrapper::get_http_version_buffer()
{
  return message_buffer_.get_status_line_3();
}

read_write_buffer& request_buffer_wrapper::get_field_value_n(unsigned int n)
{
  return message_buffer_.get_field_value(n);
}

bool request_buffer_wrapper::get_header_with_name_index(char* header_name, unsigned int& index)
{
  return message_buffer_.get_header_index_by_name(header_name, index);
}

request_buffer_wrapper::VERSION request_buffer_wrapper::get_http_version()
{
  if(message_buffer_.get_status_line_3().equals("HTTP/1.1")){
    return HTTP1_1;
  }
  if(message_buffer_.get_status_line_3().equals("HTTP/1.0")){
    return HTTP1;
  }
  return INVALID;
}

}
}
