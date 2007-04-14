//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_LINE_RECEIVER_HPP
#define SF_LINE_RECEIVER_HPP

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace switchflow{
namespace http{

class i_line_receiver
{
 public:
  virtual STATUS receive_line(asio::const_buffer buffer, bool b_complete) = 0;
};

} // namespace httplib
} // namespace switchflow
#endif
