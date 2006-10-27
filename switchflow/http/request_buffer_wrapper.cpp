//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "request_buffer_wrapper.hpp"

namespace http{
  

HTTPRequestBufferWrapper::HTTPRequestBufferWrapper(message_buffer& messageBuffer):messageBuffer_(messageBuffer)
{
}

HTTPRequestBufferWrapper::~HTTPRequestBufferWrapper()
{
}

read_write_buffer& HTTPRequestBufferWrapper::getMethod()
{
  return messageBuffer_.get_status_line_1();
}

read_write_buffer& HTTPRequestBufferWrapper::getURI()
{
  return messageBuffer_.get_status_line_2();
}

read_write_buffer& HTTPRequestBufferWrapper::getHTTPVersionBuffer()
{
  return messageBuffer_.get_status_line_3();
}

read_write_buffer& HTTPRequestBufferWrapper::getFieldValueN(unsigned int n)
{
  return messageBuffer_.get_field_value(n);
}

bool HTTPRequestBufferWrapper::getHeaderWithNameIndex(char* headerName, unsigned int& index)
{
  return messageBuffer_.get_header_index_by_name(headerName, index);
}

HTTPRequestBufferWrapper::VERSION HTTPRequestBufferWrapper::getHTTPVersion()
{
  if(messageBuffer_.get_status_line_3().equals("HTTP/1.1")){
    return HTTP1_1;
  }
  if(messageBuffer_.get_status_line_3().equals("HTTP/1.0")){
    return HTTP1;
  }
  return INVALID;
}

}
