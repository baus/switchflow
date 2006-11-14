//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "non_validating_proxy_stream_handler.hpp"

socketlib::STATUS non_validating_proxy_stream_handler::process_data(read_write_buffer& buf)
{
  return get_proxy_stream_interface()->forward(buf);
}

non_validating_proxy_stream_handler::non_validating_proxy_stream_handler()
  : i_proxy_stream_handler()
{
}

non_validating_proxy_stream_handler* non_validating_proxy_stream_handler::clone()
{
  return 0; // non_validating_proxy_stream_handler;
}
