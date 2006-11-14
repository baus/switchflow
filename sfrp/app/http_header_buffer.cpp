//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "http_header_buffer.hpp"


http_header_buffer::http_header_buffer(unsigned int max_name_length, 
           unsigned int max_value_length):name_(max_name_length), 
                value_(max_value_length)
{
  
}

buffer& http_header_buffer::get_name()
{
  return name_;
}

buffer& http_header_buffer::get_value()
{
  return value_;
}

void http_header_buffer::reset()
{
  name_.reset();
  value_.reset();
}

void http_header_buffer::append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  name_.append_from_buffer(begin, end);
}

void http_header_buffer::append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  value_.append_from_buffer(begin, end);
}

bool http_header_buffer::name_equals(char* compare_string)
{
  return name_.equals(compare_string);
}

bool http_header_buffer::value_equals(char* compare_string)
{
  return value_.equals(compare_string);
}


