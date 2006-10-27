//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "HTTPHeaderBuffer.hpp"


HTTPHeaderBuffer::HTTPHeaderBuffer(unsigned int maxNameLength, 
           unsigned int maxValueLength):name_(maxNameLength), 
                value_(maxValueLength)
{
  
}

Buffer& HTTPHeaderBuffer::getName()
{
  return name_;
}

Buffer& HTTPHeaderBuffer::getValue()
{
  return value_;
}

void HTTPHeaderBuffer::reset()
{
  name_.reset();
  value_.reset();
}

void HTTPHeaderBuffer::appendToName(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  name_.appendFromBuffer(begin, end);
}

void HTTPHeaderBuffer::appendToValue(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  value_.appendFromBuffer(begin, end);
}

bool HTTPHeaderBuffer::nameEquals(char* compareString)
{
  return name_.equals(compareString);
}

bool HTTPHeaderBuffer::valueEquals(char* compareString)
{
  return value_.equals(compareString);
}


