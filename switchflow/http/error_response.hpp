//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

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
    Raw_buffer start_line_1;
    Raw_buffer start_line_2;
    Raw_buffer start_line_3;

    Raw_buffer connection_field_name;
    Raw_buffer connection_field_value;
    
    header_buffer field;
    std::list<header_buffer*> field_list;
  };

  static static_buffers buffers_;
};

}

#endif // ERROR_RESPONSE_HPP
