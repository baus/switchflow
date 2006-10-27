//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "header_buffer.hpp"


header_buffer::header_buffer(unsigned int maxNameLength, 
                                   unsigned int maxValueLength):name_(maxNameLength), 
                                                                value_(maxValueLength)
{
  
}

header_buffer::header_buffer()
{
  
}

read_write_buffer& header_buffer::getName()
{
  return name_;
}

read_write_buffer& header_buffer::getValue()
{
  return value_;
}

void header_buffer::reset()
{
  name_.reset();
  value_.reset();
}

void header_buffer::append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  name_.appendFromBuffer(begin, end);
}

void header_buffer::append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  value_.appendFromBuffer(begin, end);
}

bool header_buffer::name_equals(const char* compareString)
{
  return name_.equals(compareString);
}

bool header_buffer::value_equals(const char* compareString)
{
  return value_.equals(compareString);
}


