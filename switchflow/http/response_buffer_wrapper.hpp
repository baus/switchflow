//
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_HTTPRESPONSEBUFFERWRAPPER_HPP
#define SSD_HTTPRESPONSEBUFFERWRAPPER_HPP

#include "message_buffer.hpp"

namespace http{
  
class HTTPResponseBufferWrapper
{
public:
  HTTPResponseBufferWrapper(message_buffer& messageBuffer);
  virtual ~HTTPResponseBufferWrapper();

  read_write_buffer& getHTTPVersion();
  read_write_buffer& getStatusCode();
  read_write_buffer& getReasonPhrase();
  
private:
  message_buffer& m_messageBuffer;
};

}
#endif // HTTPREQUESTBUFFERWRAPPER_HPP
