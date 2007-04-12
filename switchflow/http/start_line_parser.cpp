//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <algorithm>

#include <util/logger.hpp>

#include "http.hpp"
#include "start_line_parser.hpp"
#include "i_start_line_receiver.hpp"

namespace switchflow{
namespace http{

  
start_line_parser::start_line_parser(i_start_line_receiver& receiver,
                                     size_t token1_max_length,
                                     size_t token2_max_length,
                                     size_t token3_max_length):
  state_(STATE_TOKEN_1),
  current_length_(0),
  receiver_(receiver),
  token1_max_length_(token1_max_length),
  token2_max_length_(token2_max_length),
  token3_max_length_(token3_max_length)
{
}

void start_line_parser::reset()
{
  state_ = STATE_TOKEN_1;
  current_length_ = 0;
}

std::pair<STATUS, asio::const_buffer> start_line_parser::parse_start_line(asio::const_buffer buffer, PARSE_OPTION option)
{

  std::pair<STATUS, asio::const_buffer> return_value = std::make_pair(INCOMPLETE, buffer);

  size_t token_max_length = 0;
  while(asio::buffer_size(buffer) > 0){
    switch(state_){
      case STATE_TOKEN_1:
        token_max_length = token1_max_length_;

      case STATE_TOKEN_2:
        token_max_length = token2_max_length_;

      case STATE_TOKEN_3:
        token_max_length = token3_max_length_;

        return_value = parse_token(buffer, token_max_length, option);
        buffer = return_value.second;
        if(return_value.first != COMPLETE){
          return return_value;
        }
        else{
          return_value.first = INCOMPLETE;
        }
        break;
        
      case STATE_LF:
        return_value = parse_char(buffer, LF);
        return return_value;
        break;
    }
  }
  return return_value;
}

std::pair<STATUS, asio::const_buffer> start_line_parser::parse_token(asio::const_buffer buffer,
                                                                     size_t max_length,
                                                                     PARSE_OPTION option)
{
  parse_result result = http::parse_token(buffer,
                                          current_length_,
                                          max_length,
                                          get_token_delimiters(option));
  
  if(result.status == OVERFLOW || result.status == INVALID){
    return std::make_pair(result.status, result.result_buffer);
  }

  current_length_ += asio::buffer_size(result.result_buffer);
    
  STATUS return_value = set_token(result.result_buffer, result.status == COMPLETE);

  if(return_value == DATAOVERFLOW || return_value == INVALID){
    return std::make_pair(result.status, result.remaining_buffer);
  }
  else if(result.status == COMPLETE){
    next_state(result.delimiter);
  }
  return std::make_pair(result.status, result.remaining_buffer);
}


void start_line_parser::next_state(char delimiter)
{
  switch(state_){
    case STATE_TOKEN_1:
      CHECK_CONDITION_VAL(delimiter == SP, "Invalid delimiter used to change status TOKEN_1 state", delimiter);
      state_ = STATE_TOKEN_2;
      break;
    case STATE_TOKEN_2:
      if(delimiter == CR){
        //
        // This is a pseudo - invalid status line that doesn't include the reason token.
        // Apache 1.3 generate status lines like this.  This should only happen when
        // loose parsing is allowed.
        state_ = STATE_LF;
      }
      else if(delimiter == SP){
        state_ = STATE_TOKEN_3;
      }
      else{
        CHECK_CONDITION_VAL(false, "Invalid delimiter use to change status TOKEN_2 state", delimiter);
      }
      break;
    case STATE_TOKEN_3:
      CHECK_CONDITION_VAL(delimiter == CR, "Invalid delimiter use to change status TOKEN_3 state", delimiter);
      state_ = STATE_LF;
      break;
    case STATE_LF:
      state_ = STATE_TOKEN_1;
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state", state_);
  }
  current_length_ = 0;
}

STATUS start_line_parser::set_token(asio::const_buffer buffer, bool b_complete)
{
  STATUS return_value;
  switch(state_){
    case STATE_TOKEN_1:
      return_value = receiver_.start_line_token1(buffer, b_complete);
      break;
    case STATE_TOKEN_2:
      return_value = receiver_.start_line_token2(buffer, b_complete);
      break;
    case STATE_TOKEN_3:
      return_value = receiver_.start_line_token3(buffer, b_complete);
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state in set status token", state_);
      
  }

  return return_value;
}
  
std::set<char>& start_line_parser::get_token_delimiters(PARSE_OPTION option)
{
  switch(state_){
    case STATE_TOKEN_1:
      return s_space_delimiters;
      break;
    case STATE_TOKEN_2:
      if(option == STRICT){
        return s_space_delimiters;
      }
      else{
        return s_space_endline_delimiters;
      }
      break;
    case STATE_TOKEN_3:
      return s_endline_delimiters;
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state to retrieve token delimiters", state_);
  }

}

}// namespace httplib
}// namespace switchflow
