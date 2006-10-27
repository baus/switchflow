//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_HTTPHEADERBUFFER_HPP
#define SSD_HTTPHEADERBUFFER_HPP

#include <Buffer.h>

class HTTPHeaderBuffer
{
 public:
  HTTPHeaderBuffer(unsigned int maxNameLength, unsigned int maxValueLength);
  void reset();
  Buffer& getName();
  Buffer& getValue();
  void appendToName(RawBuffer::iterator begin, RawBuffer::iterator end);
  void appendToValue(RawBuffer::iterator begin, RawBuffer::iterator end);
  bool nameEquals(char* compareString);
  bool valueEquals(char* compareString);
 private:
  Buffer name_;
  Buffer value_;
};


#endif // HTTPHEADERBUFFER_HPP
