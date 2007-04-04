//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_HTTP_HEADERBUFFER_HPP
#define SSD_HTTP_HEADERBUFFER_HPP

#include <buffer.hpp>

class http_header_buffer
{
 public:
  http_header_buffer(unsigned int max_name_length, unsigned int max_value_length);
  void reset();
  buffer& get_name();
  buffer& get_value();
  void append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end);
  void append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end);
  bool name_equals(char* compare_string);
  bool value_equals(char* compare_string);
 private:
  buffer name_;
  buffer value_;
};


#endif // HTTPHEADERBUFFER_HPP
