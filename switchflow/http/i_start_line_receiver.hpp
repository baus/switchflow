//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_START_LINE_RECEIVER_HPP
#define SF_START_LINE_RECEIVER_HPP

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace switchflow{
namespace http{

class i_start_line_receiver
{
 public:
  virtual STATUS start_line_token1(asio::const_buffer buffer, bool b_complete) = 0;
  virtual STATUS start_line_token2(asio::const_buffer buffer, bool b_complete) = 0;
  virtual STATUS start_line_token3(asio::const_buffer buffer, bool b_complete) = 0;
  
};

} // namespace httplib
} // namespace switchflow
#endif
