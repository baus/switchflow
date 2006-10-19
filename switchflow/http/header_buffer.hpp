// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_HTTPHEADERBUFFER_HPP
#define SSD_HTTPHEADERBUFFER_HPP

#include <util/read_write_buffer.hpp>

class header_buffer
{
 public:
  header_buffer(unsigned int maxNameLength, unsigned int maxValueLength);
  header_buffer();
  void reset();
  read_write_buffer& getName();
  read_write_buffer& getValue();
  void append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end);
  void append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end);
  bool name_equals(const char* compareString);
  bool value_equals(const char* compareString);
 private:
  read_write_buffer m_name;
  read_write_buffer m_value;
};


#endif // HTTPHEADERBUFFER_HPP
