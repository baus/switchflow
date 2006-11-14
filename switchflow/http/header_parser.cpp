//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>

#include <algorithm>

#include <util/logger.hpp>

#include "http.hpp"
#include "header_parser.hpp"
#include "i_header_receiver.hpp"

namespace http{
  
http_header_parser::http_header_parser(i_header_receiver* p_header_receiver):
  current_state_(STATUS_LINE),
  status_line_state_(STATUS_LINE_TOKEN_1),
  current_length_(0),
  p_header_receiver_(p_header_receiver),
  chunk_size_buffer_(10)
{
  
}

STATUS http_header_parser::parse_status_token(read_write_buffer& buffer,
                                          unsigned int begin_offset,
                                          unsigned int& end_offset,
                                          PARSE_OPTION option)
{
  STATUS forward_status = COMPLETE;
  STATUS return_value = parse_token(buffer,
                                  begin_offset,
                                  end_offset,
                                  current_length_,
                                  MAX_FIELD_LENGTH,
                                  get_status_token_delimiters(option));
  
  if(return_value == COMPLETE){
    return_value = receive_status_line_token(buffer, begin_offset, end_offset - 1, true);
    if(DATAOVERFLOW != return_value && return_value != INVALID){
      next_status_line_state(buffer[end_offset - 1]);
    }
    else{
      log_info("Invalid status line token", status_line_state_);
    }
  }
  else if(return_value == INCOMPLETE){
    forward_status =  receive_status_line_token(buffer,
                                            begin_offset,
                                            end_offset,
                                            false);
    if(DATAOVERFLOW == forward_status || INVALID == forward_status){
      return_value = forward_status;
    }   
  }
  else{
    log_info("Overflow status line token", status_line_state_);
    return_value = INVALID;
  }
  
  return return_value;
  
}

STATUS http_header_parser::parse_status_line(read_write_buffer& buffer, PARSE_OPTION option)
{
  
  //
  // Is this the correct initialization?
  //
  STATUS return_value = COMPLETE;
  STATUS forward_status = COMPLETE;
  bool buffer_parse_complete = false;
  unsigned int current_offset = buffer.get_process_position();
  unsigned int begin_offset;
  bool finished = false;
  
  while(!buffer_parse_complete){
    begin_offset = current_offset;
    switch(status_line_state_){
      case STATUS_LINE_TOKEN_1:
      case STATUS_LINE_TOKEN_2:
      case STATUS_LINE_TOKEN_3:
        return_value = parse_status_token(buffer, begin_offset, current_offset, option);
        buffer_parse_complete = return_value != COMPLETE;
        break;
        
      case STATUS_LINE_LF:
        return_value = parse_char(buffer, begin_offset, current_offset, LF);
        buffer_parse_complete = true;
        break;
    }
  }

  buffer.set_process_position(current_offset);
  //
  // Since we are not writting directly out of the buffer,
  // we can mark the buffer as written even though it really
  // is never written.  If we ever want to write headers directly
  // and not buffer the  (highly unlikely), then this would be
  // incorrect.  The layer receiving the parsed buffer would
  // set the write flag.
  //
  // But for now this works as intended.
  //
  buffer.set_write_position(current_offset);

  return return_value;
}

STATUS http_header_parser::parse_headers(read_write_buffer& buffer, PARSE_OPTION option)
{

  //
  // Is this the correct initialization?
  //
  STATUS return_value = COMPLETE;
  bool buffer_parse_complete = false;
  unsigned int current_offset = buffer.get_process_position();
  unsigned int begin_offset;
  bool finished = false;

  while(!buffer_parse_complete){
    begin_offset = current_offset;
    switch(current_state_){
      case STATUS_LINE:
        return_value = parse_status_line(buffer, option);
        if(return_value == COMPLETE){
          current_offset = buffer.get_process_position();
          begin_offset = current_offset;
          transition_to_state(END_HEADERS_CR);
        }
        else{
          buffer_parse_complete = true;
        }
        break;
        
      case END_HEADERS_CR:
        return_value = parse_char(buffer, begin_offset, current_offset, CR);
        if(return_value == COMPLETE){
          transition_to_state(END_HEADERS_LF);
        }
        else if(return_value == INVALID){
          transition_to_state(FIELD_NAME);
        }
        else{
          buffer_parse_complete = true;
        }
        break;
      case END_HEADERS_LF:
        return_value = parse_char(buffer, begin_offset, current_offset, LF);
        buffer_parse_complete = true;
        if(return_value == COMPLETE){
          if(COMPLETE != (return_value = p_header_receiver_->end_fields())){
            break;
          }
          //
          // Successfully parsed all headers
          //
          finished = true;
          break;
        }
        break;
      case FIELD_NAME:
        return_value = parse_token(buffer, begin_offset, current_offset, current_length_, MAX_FIELD_LENGTH, s_field_delimiters);
        if(return_value == COMPLETE){
          if(DATAOVERFLOW == 
             p_header_receiver_->set_field_name(buffer, begin_offset, current_offset - 1, true)){
            buffer_parse_complete = true;
            return_value =  DATAOVERFLOW;
          }
          else{
            transition_to_state(FIELD_VALUE_LEADING_LWS);
          }
        }
        else{
          if(return_value == INCOMPLETE){
            if(DATAOVERFLOW == p_header_receiver_->set_field_name(buffer, 
                                                               begin_offset,
                                                               current_offset, 
                                                               false)){
              buffer_parse_complete = true;
              return_value = DATAOVERFLOW;
            }
          }
          buffer_parse_complete = true;
        }
        break;
        
      case FIELD_VALUE_LEADING_LWS:
        return_value = parse_equal(buffer, begin_offset, current_offset, current_length_, MAX_FIELD_LENGTH, s_white_space);
        if(return_value == COMPLETE){
          transition_to_state(FIELD_VALUE_CONTENT, NOT_RESET_CURRENT_LENGTH);
        }
        else{
          buffer_parse_complete = true;
        }
        break;

      case FIELD_VALUE_LWS_LF:
        return_value = parse_char(buffer, begin_offset, current_offset, LF);
        if(return_value == COMPLETE){
          transition_to_state(FIELD_VALUE_LWS);
        }
        else{
          buffer_parse_complete = true;
        }
        break;
      case FIELD_VALUE_LWS:
        return_value = parse_equal(buffer, begin_offset, current_offset, current_length_, MAX_FIELD_LENGTH, s_white_space);
        if(return_value == COMPLETE){
          if(current_offset == begin_offset){
            //
            // empty, see if we are at the end of headers, then don't get the next header..
            // but before we leave now is a good time to figure out how to parse the message
            // body...  This is the ugly, stateful part of HTTP parsing (ie the parsing of the
            // remainder of the message is determined by the message itself).  
            transition_to_state(END_HEADERS_CR);
          }
          else{
            //
            // Looks like a line continuation, back to parsing the value
            //
            transition_to_state(FIELD_VALUE_CONTENT);
          }
        }
        else{
          buffer_parse_complete = true;
        }
        break;
      case FIELD_VALUE_CONTENT:
        return_value = parse_token(buffer,
                                 begin_offset,
                                 current_offset,
                                 current_length_,
                                 MAX_FIELD_LENGTH,
                                 s_endline_delimiters);
      
        if(return_value == COMPLETE){
          //
          // This doesn't deal with line continuations correctly.  I'm not sure if we are looking a line
          // completion.  Sending true here isn't totally acurate.  Might get two trues back to back in the 
          // line continuation case...
          //
          if(DATAOVERFLOW == p_header_receiver_->set_field_value(buffer, begin_offset, current_offset - 1, true)){
            buffer_parse_complete = true;
            return_value = DATAOVERFLOW;
          }
          transition_to_state(FIELD_VALUE_LWS_LF);
        }
        else if(return_value == INCOMPLETE){
          if(DATAOVERFLOW == p_header_receiver_->set_field_value(buffer, begin_offset, current_offset, false)){
            buffer_parse_complete = true;
            return_value = DATAOVERFLOW;
          }
          else{
            buffer_parse_complete = true;
          }
        }
        else{
          buffer_parse_complete = true;
        }
        break;
      default:
        assert(false);
        break;
    }
  }
  buffer.set_process_position(current_offset);
  //
  // Since we are not writing directly out of the buffer,
  // we can mark the buffer as written even though it really
  // is never written.  If we ever want to write headers directly
  // and not buffer then (highly unlikely), then this would be
  // incorrect.  The layer receiving the parsed buffer would
  // set the write flag.
  //
  // But for now this works as intended.
  //
  buffer.set_write_position(current_offset);
  if(return_value == COMPLETE && !finished){
    return INCOMPLETE;
  }
  return return_value;
}

void http_header_parser::next_status_line_state(char delimiter)
{
  switch(status_line_state_){
    case STATUS_LINE_TOKEN_1:
      CHECK_CONDITION_VAL(delimiter == SP, "invalid delimiter used to change status TOKEN_1 state", delimiter);
      status_line_state_ = STATUS_LINE_TOKEN_2;
      break;
    case STATUS_LINE_TOKEN_2:
      if(delimiter == CR){
        //
        // This is a pseudo - invalid status line that doesn't include the reason token.
        // Apache 1.3 generate status lines like this.  This should only happen when
        // loose parsing is allowed.
        status_line_state_ = STATUS_LINE_LF;
      }
      else if(delimiter == SP){
        status_line_state_ = STATUS_LINE_TOKEN_3;
      }
      else{
        CHECK_CONDITION_VAL(false, "Invalid delimiter use to change status TOKEN_2 state", delimiter);
      }
      break;
    case STATUS_LINE_TOKEN_3:
      CHECK_CONDITION_VAL(delimiter == CR, "Invalid delimiter use to change status TOKEN_3 state", delimiter);
      status_line_state_ = STATUS_LINE_LF;
      break;
    case STATUS_LINE_LF:
      status_line_state_ = STATUS_LINE_TOKEN_1;
      break;
    default:
      CHECK_CONDITION_VAL(false, "invalid start line state", status_line_state_);
  }
  current_length_ = 0;
}

void HTTPHeader_parser::transition_to_state(PARSE_STATE new_state,
                                         TRANSITION_CURRENT_LENGTH_OPTION
                                         reset_option)
{
  current_state_ = new_state;
  if(reset_option == RESET_CURRENT_LENGTH){
    current_length_ = 0;
  }
}

void http_header_parser::reset()
{
  status_line_state_ = STATUS_LINE_TOKEN_1;
  transition_to_state(STATUS_LINE);
}

STATUS HTTPHeader_parser::receive_status_line_token(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)
{
  STATUS return_value;
  switch(status_line_state_){
    case STATUS_LINE_TOKEN_1:
      return_value = p_header_receiver_->start_line_token1(buffer, i_begin, i_end, b_complete);
      break;
    case STATUS_LINE_TOKEN_2:
      return_value = p_header_receiver_->start_line_token2(buffer, i_begin, i_end, b_complete);
      break;
    case STATUS_LINE_TOKEN_3:
      return_value = p_header_receiver_->start_line_token3(buffer, i_begin, i_end, b_complete);
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state to receieve status token", status_line_state_);
      
  }

  return return_value;
  
}
  
std::set<char>& http_header_parser::get_status_token_delimiters(PARSE_OPTION option)
{
  switch(status_line_state_){
    case STATUS_LINE_TOKEN_1:
      return s_space_delimiters;
      break;
    case STATUS_LINE_TOKEN_2:
      if(option == STRICT){
        return s_space_delimiters;
      }
      else{
        return s_space_endline_delimiters;
      }
      break;
    case STATUS_LINE_TOKEN_3:
      return s_endline_delimiters;
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state to retrieve token delimiters", status_line_state_);
  }

}

}// namespace httplib
