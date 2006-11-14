//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "response_buffer_wrapper.hpp"

namespace http{
  
//
//
http_response_buffer_wrapper::http_response_buffer_wrapper(message_buffer& message_buffer):message_buffer_(message_buffer)
{
}

http_response_buffer_wrapper::~http_response_buffer_wrapper()
{
}

read_write_buffer& http_response_buffer_wrapper::get_http_version()
{
  return message_buffer_.get_status_line_1();
}

read_write_buffer& http_response_buffer_wrapper::get_status_code()
{
  return message_buffer_.get_status_line_2();
}

read_write_buffer& http_response_buffer_wrapper::get_reason_phrase()
{
  return message_buffer_.get_status_line_3();
}

}
