//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/logger.hpp>
#include <http/http.hpp>

#include "static_strings.hpp"

#include "header_writer.hpp"


namespace switchflow{
namespace http{
  
header_writer::header_writer():message_buffer_(0),
                               space_buf_(&strings_.space_),
                               endline_buf_(&strings_.endline_),
                               field_sep_(&strings_.fieldsep_),
                               p_dest_(0)
{
}

void header_writer::reset(message_buffer& message,
                          socketlib::connection& dest)
{
  p_dest_ = &dest;
  write_header_state_ = START_LINE_TOKEN1;
  message_buffer_ = &message;
  current_dump_header_ = 0;
  space_buf_.reset();
  endline_buf_.reset();
  field_sep_.reset();
}

header_writer::~header_writer()
{
}

socketlib::STATUS header_writer::write_header()
{
  socketlib::STATUS status;

  for(;;){
    if(write_header_state_ == FIELD_NAME){
      if(current_dump_header_ == message_buffer_->get_num_fields()){
        write_header_state_ = END_FIELDS;
      }
    }
    
    read_write_buffer& buffer_to_write = get_header_buffer_to_write(static_cast<WRITE_HEADER_STATE>(write_header_state_));
    status = p_dest_->non_blocking_write(buffer_to_write);
    if(status == socketlib::INCOMPLETE){
      return status;
    }
    if(status == socketlib::COMPLETE){
      space_buf_.reset();
      endline_buf_.reset();
      field_sep_.reset();
      if(write_header_state_ == FIELD_SEPERATOR){
        write_header_state_ = FIELD_NAME;
        ++current_dump_header_;
      }
      else if(write_header_state_ == END_FIELDS){
        //
        // All done dumping headers.
        return socketlib::COMPLETE;
      }
      else{
        //
        // just go to the next sequential state
        ++write_header_state_; 
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

read_write_buffer& header_writer::get_header_buffer_to_write(WRITE_HEADER_STATE header_state)
{
  switch(write_header_state_){
    case START_LINE_TOKEN1:
      return message_buffer_->get_status_line_1();
      break;
    case START_LINE_SEPERATOR1:
      return space_buf_;
      break;
    case START_LINE_TOKEN2:
      return message_buffer_->get_status_line_2();
      break;
    case START_LINE_SEPERATOR2:
      return space_buf_;
      break;
    case START_LINE_TOKEN3:
      return message_buffer_->get_status_line_3();
      break;
    case START_LINE_END:
      return endline_buf_;
      break;
    case FIELD_NAME:
      return message_buffer_->get_field_name(current_dump_header_);
      break;
    case FIELD_VALUE_SEPERATOR:
      return field_sep_;
      break;
    case FIELD_VALUE:
      return message_buffer_->get_field_value(current_dump_header_);
      break;
    case FIELD_SEPERATOR:
      return endline_buf_;
      break;
    case END_FIELDS:
      return endline_buf_;
      break;
  };
}

} //namespace http
} //namespace switchflow
