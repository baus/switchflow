//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>

#include <string>

#include <util/logger.hpp>

#include "proxy_handler.h"
#include "proxy_stream_interface.h"


//
// @class proxy_stream_interface proxystreaminterface.h
// @author <a href="mailto:christopher@baus.net">christopher</a>
namespace proxylib{

proxy_stream_interface::proxy_stream_interface():
  p_proxy_handler_(0),
  p_src_data_(0), 
  p_dest_data_(0)
{
}

bool proxy_stream_interface::is_dest_disconnected()
{
  assert(p_dest_data_ == NULL);
  return p_dest_data_->state() == socketlib::connection::NOT_CONNECTED;
}

socketlib::STATUS proxy_stream_interface::forward(read_write_buffer& buf)
{
  return p_dest_data_->non_blocking_write(buf);
}
    
void proxy_stream_interface::reset(proxy_handler* p_proxy_handler, 
                                 socketlib::connection* p_src_data,
                                 socketlib::connection* p_dest_data)
{
//  assert(p_proxy_handler != 0);
//  assert(p_src_data != 0);
//  assert(p_dest_data != 0);
  
  p_src_data_ = p_src_data;
  p_dest_data_ = p_dest_data;
  p_proxy_handler_ = p_proxy_handler;
}

const char* proxy_stream_interface::get_src_address()
{
  return p_src_data_->get_ip_addr();
}

const char* proxy_stream_interface::get_dest_address()
{
  return p_dest_data_->get_ip_addr();
}

void proxy_stream_interface::flush()
{
  return p_dest_data_->flush();
}

pipeline_data_queue* proxy_stream_interface::get_pipeline_data_queue()
{
  return &p_proxy_handler_->pipeline_data_queue_;
}


socketlib::connection* proxy_stream_interface::get_dest()
{
  return p_dest_data_;
}

} // namespace proxylib
