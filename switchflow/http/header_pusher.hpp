// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_HEADER_PUSHER_HPP
#define SSD_HEADER_PUSHER_HPP

#include <socketlib/status.hpp>
#include <socketlib/connection.hpp>

#include "message_buffer.hpp"

namespace http{
  
class header_pusher
{
public:
  header_pusher();
  virtual ~header_pusher();
  socketlib::STATUS push_header();
  void reset(message_buffer& message, socketlib::connection& dest);

  
private:
  //
  // The current state of pushing headers to 
  // next up stream HTTP processor.  push_header
  // can return from any of these states.
  enum PUSH_HEADER_STATE{
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
  read_write_buffer m_spaceBuf;
  read_write_buffer m_endlineBuf;
  read_write_buffer m_fieldSep;

  read_write_buffer& get_header_buffer_to_push(PUSH_HEADER_STATE header_state);

  unsigned int current_dump_header_;
  unsigned int push_header_state_;
  message_buffer* message_buffer_;
  socketlib::connection* p_dest_;
};

}

#endif // HEADER_PUSHER_HPP
