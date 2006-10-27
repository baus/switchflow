// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "HTTPHeaderBuffer.hpp"


HTTPHeaderBuffer::HTTPHeaderBuffer(unsigned int maxNameLength, 
           unsigned int maxValueLength):m_name(maxNameLength), 
                m_value(maxValueLength)
{
  
}

Buffer& HTTPHeaderBuffer::getName()
{
  return m_name;
}

Buffer& HTTPHeaderBuffer::getValue()
{
  return m_value;
}

void HTTPHeaderBuffer::reset()
{
  m_name.reset();
  m_value.reset();
}

void HTTPHeaderBuffer::appendToName(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  m_name.appendFromBuffer(begin, end);
}

void HTTPHeaderBuffer::appendToValue(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  m_value.appendFromBuffer(begin, end);
}

bool HTTPHeaderBuffer::nameEquals(char* compareString)
{
  return m_name.equals(compareString);
}

bool HTTPHeaderBuffer::valueEquals(char* compareString)
{
  return m_value.equals(compareString);
}


