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
HTTPResponseBufferWrapper::HTTPResponseBufferWrapper(message_buffer& messageBuffer):messageBuffer_(messageBuffer)
{
}

HTTPResponseBufferWrapper::~HTTPResponseBufferWrapper()
{
}

read_write_buffer& HTTPResponseBufferWrapper::getHTTPVersion()
{
  return messageBuffer_.get_status_line_1();
}

read_write_buffer& HTTPResponseBufferWrapper::getStatusCode()
{
  return messageBuffer_.get_status_line_2();
}

read_write_buffer& HTTPResponseBufferWrapper::getReasonPhrase()
{
  return messageBuffer_.get_status_line_3();
}

}
