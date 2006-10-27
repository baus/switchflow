//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <util/logger.hpp>
#include <http/http.hpp>

#include "static_strings.hpp"

#include "header_pusher.hpp"


namespace http{
  
header_pusher::header_pusher():message_buffer_(0),
                               m_spaceBuf(&strings_.space_),
                               m_endlineBuf(&strings_.endline_),
                               m_fieldSep(&strings_.fieldsep_),
                               p_dest_(0)
{
}

void header_pusher::reset(message_buffer& message,
                          socketlib::connection& dest)
{
  p_dest_ = &dest;
  push_header_state_ = START_LINE_TOKEN1;
  message_buffer_ = &message;
  current_dump_header_ = 0;
  m_spaceBuf.reset();
  m_endlineBuf.reset();
  m_fieldSep.reset();
}

header_pusher::~header_pusher()
{
}

socketlib::STATUS header_pusher::push_header()
{
  socketlib::STATUS status;

  for(;;){
    if(push_header_state_ == FIELD_NAME){
      if(current_dump_header_ == message_buffer_->get_num_fields()){
        push_header_state_ = END_FIELDS;
      }
    }
    
    read_write_buffer& bufferToPush = get_header_buffer_to_push(static_cast<PUSH_HEADER_STATE>(push_header_state_));
    status = p_dest_->non_blocking_write(bufferToPush);
    if(status == socketlib::INCOMPLETE){
      return status;
    }
    if(status == socketlib::COMPLETE){
      m_spaceBuf.reset();
      m_endlineBuf.reset();
      m_fieldSep.reset();
      if(push_header_state_ == FIELD_SEPERATOR){
        push_header_state_ = FIELD_NAME;
        ++current_dump_header_;
      }
      else if(push_header_state_ == END_FIELDS){
        //
        // All done dumping headers.
        return socketlib::COMPLETE;
      }
      else{
        //
        // just go to the next sequential state
        ++push_header_state_; 
      }
    }
    else if(status == socketlib::DEST_CLOSED){
      return status;
    }
    else{
      CHECK_CONDITION(false, "forward() returned unknown status");
    }
  }
}

read_write_buffer& header_pusher::get_header_buffer_to_push(PUSH_HEADER_STATE headerState)
{
  switch(push_header_state_){
    case START_LINE_TOKEN1:
      return message_buffer_->get_status_line_1();
      break;
    case START_LINE_SEPERATOR1:
      return m_spaceBuf;
      break;
    case START_LINE_TOKEN2:
      return message_buffer_->get_status_line_2();
      break;
    case START_LINE_SEPERATOR2:
      return m_spaceBuf;
      break;
    case START_LINE_TOKEN3:
      return message_buffer_->get_status_line_3();
      break;
    case START_LINE_END:
      return m_endlineBuf;
      break;
    case FIELD_NAME:
      return message_buffer_->get_field_name(current_dump_header_);
      break;
    case FIELD_VALUE_SEPERATOR:
      return m_fieldSep;
      break;
    case FIELD_VALUE:
      return message_buffer_->get_field_value(current_dump_header_);
      break;
    case FIELD_SEPERATOR:
      return m_endlineBuf;
      break;
    case END_FIELDS:
      return m_endlineBuf;
      break;
  };
}

}
