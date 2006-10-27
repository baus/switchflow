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
                               spaceBuf_(&strings_.space_),
                               endlineBuf_(&strings_.endline_),
                               fieldSep_(&strings_.fieldsep_),
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
  spaceBuf_.reset();
  endlineBuf_.reset();
  fieldSep_.reset();
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
      spaceBuf_.reset();
      endlineBuf_.reset();
      fieldSep_.reset();
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
      return spaceBuf_;
      break;
    case START_LINE_TOKEN2:
      return message_buffer_->get_status_line_2();
      break;
    case START_LINE_SEPERATOR2:
      return spaceBuf_;
      break;
    case START_LINE_TOKEN3:
      return message_buffer_->get_status_line_3();
      break;
    case START_LINE_END:
      return endlineBuf_;
      break;
    case FIELD_NAME:
      return message_buffer_->get_field_name(current_dump_header_);
      break;
    case FIELD_VALUE_SEPERATOR:
      return fieldSep_;
      break;
    case FIELD_VALUE:
      return message_buffer_->get_field_value(current_dump_header_);
      break;
    case FIELD_SEPERATOR:
      return endlineBuf_;
      break;
    case END_FIELDS:
      return endlineBuf_;
      break;
  };
}

}
