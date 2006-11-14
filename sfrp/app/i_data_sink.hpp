//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef __I_DATA_SINK_H__
#define __I_DATA_SINK_H__

#include "inet_util.h"

class i_data_sink
{
public:
  bool dump_data(raw_buffer buffer, int begin, int end)
  {
    return dump_data(&buffer[0], begin, end);
  }
  bool dump_data(buffer& buffer, int begin, int end)
  {
    return dump_data(static_cast<const BYTE*>(&buffer.get_buffer()[0]), begin, end);
  }
  
  bool dump_data(const BYTE* buffer, int begin, int end)
  {
    return dump_data(reinterpret_cast<const char*>(buffer), begin, end);
  }
  
  bool dump_data(const int* buffer, int begin, int end)
  {
    return dump_data(reinterpret_cast<const char*>(buffer), begin, end);
  }
  
  virtual bool dump_data(const char* buffer, int begin, int end) = 0;
  
  virtual void flush(){}
    
  virtual ~i_data_sink(){};
  
};

#endif // __I_DATA_SINK_H__
