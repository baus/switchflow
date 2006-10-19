//
// Copyright (C) Christopher Baus.  All rights reserved.
#ifndef SSD_ERROR_RESPONSE_HPP
#define SSD_ERROR_RESPONSE_HPP

#include "message_buffer.hpp"
#include "header_cache.hpp"

namespace http{
  
class error_response
{
public:
  error_response();
  virtual ~error_response();

  message_buffer& get_message_buffer();
  void reset();
private:
  message_buffer error_response_;
  class static_buffers{
  public:
    static_buffers();
    RawBuffer start_line_1;
    RawBuffer start_line_2;
    RawBuffer start_line_3;

    RawBuffer connection_field_name;
    RawBuffer connection_field_value;
    
    header_buffer field;
    std::list<header_buffer*> field_list;
  };

  static static_buffers buffers_;
};

}

#endif // ERROR_RESPONSE_HPP
