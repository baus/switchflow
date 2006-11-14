//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "error_response.hpp"
#include "response_buffer_wrapper.hpp"

namespace http{

// 12345678 123 12345678901
// HTTP/1.1 400 Bad Request
// 12345678901 12345  
// Connection: close
error_response::error_response():
  error_response_(&error_response::buffers_.start_line_1,
                  &error_response::buffers_.start_line_2,
                  &error_response::buffers_.start_line_3,
                  error_response::buffers_.field_list)
{ reset();
}

error_response::~error_response()
{
}

message_buffer& error_response::get_message_buffer()
{
  return error_response_;
}

void error_response::reset()
{
  error_response_.reset();
}

error_response::static_buffers::static_buffers():start_line_1(8),
                                                 start_line_2(3),
                                                 start_line_3(11),
                                                 connection_field_name(10),
                                                 connection_field_value(5)
{
  init_raw_buffer(start_line_1, "HTTP/1.x");
  init_raw_buffer(start_line_2, "400");
  init_raw_buffer(start_line_3, "Bad Request");
  init_raw_buffer(connection_field_name, "Connection");
  init_raw_buffer(connection_field_value, "close");
  field.get_value().set_static_buffer(&connection_field_value);
  field.get_name().set_static_buffer(&connection_field_name);
  
  field_list.push_back(&field);
}

error_response::static_buffers error_response::buffers_;

}

