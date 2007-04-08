//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "content_length_body_parser.hpp"

namespace switchflow{
namespace http{
  
content_length_body_parser::content_length_body_parser(i_body_receiver* p_body_receiver):
  p_body_receiver_(p_body_receiver)
{
  reset(0);
}

void content_length_body_parser::reset(unsigned int message_size)
{
  parse_state_ = MESSAGE_BODY_PARSE;
  content_length_ = message_size;
  current_offset_ = 0;
  current_length_ = 0;
  
}

content_length_body_parser::~content_length_body_parser()
{
}

STATUS content_length_body_parser::parse_content_length_body(read_write_buffer& buffer)
{
  STATUS return_value = INVALID;
  bool current_buffer_processed = false;
  while(!current_buffer_processed){
    switch(parse_state_){
      case MESSAGE_BODY_PARSE:
        buffer.set_write_position(buffer.get_process_position());

        return_value = parse_n_length_buffer(buffer, current_length_, content_length_);

        buffer.set_write_end_position(buffer.get_process_position());

        if(return_value == COMPLETE){
          transition_to_state(COMPLETE_MESSAGE_BODY_FORWARD);
        }
        else if(return_value == INCOMPLETE){
          transition_to_state(INCOMPLETE_MESSAGE_BODY_FORWARD);
        }
        break;
      case INCOMPLETE_MESSAGE_BODY_FORWARD:
        return_value = p_body_receiver_->set_body(buffer, false);
        if(return_value == COMPLETE){
          transition_to_state(MESSAGE_BODY_PARSE);
          return_value = INCOMPLETE;
        }
        else if(return_value == INCOMPLETE){
          return_value = WRITE_INCOMPLETE;
        }
        current_buffer_processed = true;
        
        break;
      case COMPLETE_MESSAGE_BODY_FORWARD:
        return_value = p_body_receiver_->set_body(buffer, true);
        if(return_value == INCOMPLETE){
          return_value = WRITE_INCOMPLETE;
        }
        current_buffer_processed = true;
        break;
      default:
        CHECK_CONDITION(false, "Invalid State while parsing HTTP body.");
        break;
    }
  }
  return return_value;
}

void content_length_body_parser::transition_to_state(content_length_body_parser::PARSE_STATE new_state)
{
  parse_state_ = new_state;
}

}//namespace http
}//namespace switchflow
