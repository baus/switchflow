//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HEADER_WRITER_HPP
#define SF_HEADER_WRITER_HPP

#include <socketlib/status.hpp>
#include <socketlib/connection.hpp>

#include "message_buffer.hpp"

namespace switchflow{
namespace http{
  
class header_writer
{
public:
  header_writer();
  virtual ~header_writer();
  socketlib::STATUS write_header();
  void reset(message_buffer& message, socketlib::connection& dest);

  
private:
  //
  // The current state of writing headers to 
  // next upstream HTTP processor.  write_header
  // can return from any of these states.
  enum WRITE_HEADER_STATE{
    START_LINE_TOKEN1,
    START_LINE_SEPERATOR1,
    START_LINE_TOKEN2,
    START_LINE_SEPERATOR2,
    START_LINE_TOKEN3,
    START_LINE_END,
    FIELD_NAME,
    FIELD_VALUE_SEPERATOR,
    FIELD_VALUE,
    FIELD_SEPERATOR,
    END_FIELDS
  };

  //
  // These can't be static, because their position 
  // pointers are used to write out buffers.  This 
  // demonstrates the problem with Buffer class.
  // The data needs to be removed from the meta data.
  read_write_buffer space_buf_;
  read_write_buffer endline_buf_;
  read_write_buffer field_sep_;

  read_write_buffer& get_header_buffer_to_write(WRITE_HEADER_STATE header_state);

  unsigned int current_dump_header_;
  unsigned int write_header_state_;
  message_buffer* message_buffer_;
  socketlib::connection* p_dest_;
};

}
}

#endif
