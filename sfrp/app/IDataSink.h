//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

/**
 * InetUtil
 *
 * © Copyright 2003 Summit Sage, LLC
 * All Rights Reserved                  
 *
 * @author Christopher Baus <christopher@summitsage.com>
 */
#ifndef __IDataSink_H__
#define __IDataSink_H__

#include "InetUtil.h"

class IDataSink
{
public:
  bool dumpData(RawBuffer buffer, int begin, int end)
  {
    return dumpData(&buffer[0], begin, end);
  }
  bool dumpData(Buffer& buffer, int begin, int end)
  {
    return dumpData(static_cast<const BYTE*>(&buffer.getBuffer()[0]), begin, end);
  }
  
  bool dumpData(const BYTE* buffer, int begin, int end)
  {
    return dumpData(reinterpret_cast<const char*>(buffer), begin, end);
  }
  
  bool dumpData(const int* buffer, int begin, int end)
  {
    return dumpData(reinterpret_cast<const char*>(buffer), begin, end);
  }
  
  virtual bool dumpData(const char* buffer, int begin, int end) = 0;
  
  virtual void flush(){}
    
  virtual ~IDataSink(){};
  
};

#endif
