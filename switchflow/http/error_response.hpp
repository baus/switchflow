//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_ERROR_RESPONSE_HPP
#define SF_ERROR_RESPONSE_HPP

#include "message_buffer.hpp"
#include "header_cache.hpp"

namespace switchflow{
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
    raw_buffer start_line_1;
    raw_buffer start_line_2;
    raw_buffer start_line_3;

    raw_buffer connection_field_name;
    raw_buffer connection_field_value;
    
    header_buffer field;
    std::list<header_buffer*> field_list;
  };

  static static_buffers buffers_;
};

} // namespace http
} // namespace switchflow

#endif // ERROR_RESPONSE_HPP
