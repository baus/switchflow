//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus. All rights reserved.
//
#ifndef HTTPREQUESTBUFFERWRAPPER_HPP
#define HTTPREQUESTBUFFERWRAPPER_HPP

#include "message_buffer.hpp"

namespace http{

class HTTPRequestBufferWrapper
{
public:
  enum VERSION{
    HTTP1,
    HTTP1_1,
    INVALID
  };
  
  HTTPRequestBufferWrapper(message_buffer& messageBuffer);
  virtual ~HTTPRequestBufferWrapper();

  read_write_buffer& getMethod();
  read_write_buffer& getURI();
  read_write_buffer& getHTTPVersionBuffer();
  VERSION getHTTPVersion();
  read_write_buffer& getFieldValueN(unsigned int n);
  bool getHeaderWithNameIndex(char* headerName, unsigned int& index);
      
private:
  message_buffer& messageBuffer_;
};

}

#endif // HTTPREQUESTBUFFERWRAPPER_HPP
