//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include "response_buffer_wrapper.hpp"

namespace http{
  
//
//
HTTPResponseBufferWrapper::HTTPResponseBufferWrapper(message_buffer& messageBuffer):m_messageBuffer(messageBuffer)
{
}

HTTPResponseBufferWrapper::~HTTPResponseBufferWrapper()
{
}

read_write_buffer& HTTPResponseBufferWrapper::getHTTPVersion()
{
  return m_messageBuffer.get_status_line_1();
}

read_write_buffer& HTTPResponseBufferWrapper::getStatusCode()
{
  return m_messageBuffer.get_status_line_2();
}

read_write_buffer& HTTPResponseBufferWrapper::getReasonPhrase()
{
  return m_messageBuffer.get_status_line_3();
}

}
