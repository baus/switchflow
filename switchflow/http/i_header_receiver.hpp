//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef HTTP_MESSAGE_RECEIVER_H__
#define HTTP_MESSAGE_RECEIVER_H__

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace http{

class i_header_receiver
{
 public:
  virtual STATUS start_line_token1(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
  virtual STATUS start_line_token2(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
  virtual STATUS start_line_token3(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
  virtual STATUS set_field_name(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
  virtual STATUS set_field_value(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
  virtual STATUS end_fields() = 0;
  
};

} // namespace httplib
#endif
