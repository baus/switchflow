// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "header_buffer.hpp"


header_buffer::header_buffer(unsigned int maxNameLength, 
                                   unsigned int maxValueLength):m_name(maxNameLength), 
                                                                m_value(maxValueLength)
{
  
}

header_buffer::header_buffer()
{
  
}

read_write_buffer& header_buffer::getName()
{
  return m_name;
}

read_write_buffer& header_buffer::getValue()
{
  return m_value;
}

void header_buffer::reset()
{
  m_name.reset();
  m_value.reset();
}

void header_buffer::append_to_name(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  m_name.appendFromBuffer(begin, end);
}

void header_buffer::append_to_value(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  m_value.appendFromBuffer(begin, end);
}

bool header_buffer::name_equals(const char* compareString)
{
  return m_name.equals(compareString);
}

bool header_buffer::value_equals(const char* compareString)
{
  return m_value.equals(compareString);
}


