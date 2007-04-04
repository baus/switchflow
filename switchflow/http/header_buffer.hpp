//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_HTTPHEADERBUFFER_HPP
#define SSD_HTTPHEADERBUFFER_HPP

#include <util/read_write_buffer.hpp>

class header_buffer
{
 public:
  header_buffer(unsigned int max_name_length, unsigned int max_value_length);
  header_buffer();
  void reset();
  read_write_buffer& get_name();
  read_write_buffer& get_value();
  void append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end);
  void append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end);
  bool name_equals(const char* compare_string);
  bool value_equals(const char* compare_string);
 private:
  read_write_buffer name_;
  read_write_buffer value_;
};


#endif // HTTPHEADERBUFFER_HPP
