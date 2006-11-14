//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef NON_VALIDATING_PROXY_STREAM_HANDLER_H__
#define NON_VALIDATING_PROXY_STREAM_HANDLER_H__

#include <proxylib/Proxy_handler.h>


class non_validating_proxy_stream_handler: public proxylib::i_proxy_stream_handler
{
 public:
  non_validating_proxy_stream_handler();
  socketlib::STATUS process_data(read_write_buffer& buf);
  non_validating_proxy_stream_handler* clone();
  virtual void reset(){}
};
#endif // NON_VALIDATING_PROXY_STREAM_HANDLER_H__

