//
//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <algorithm>

#include <util/logger.hpp>

#include "http.hpp"
#include "line_parser.hpp"
#include "i_line_receiver.hpp"

namespace switchflow{
namespace http{

  line_parser::line_parser(i_line_receiver& receiver,
                           size_t max_length):receiver_(receiver),
                                              current_length_(0),
                                              max_length_(max_length),
                                              state_(PARSE_LINE)
  {
    
  }

  void line_parser::reset()
  {
    current_length_ = 0;
    state_ = PARSE_LINE;
  }
    
  parse_result line_parser::parse_line(asio::const_buffer buffer)
  {
    asio::const_buffer forward_buffer;

    parse_result return_value;
    std::pair<STATUS, asio::const_buffer> parse_char_result;
      
    while(true){
      switch(state_)
      {
        case PARSE_LINE:
          return_value = parse_token(buffer,
                                     current_length_,
                                     max_length_,
                                     s_endline_delimiters);
          current_length_ += asio::buffer_size(return_value.result_buffer);
          
          forward_buffer = return_value.result_buffer;
          buffer = return_value.remaining_buffer;
          if(return_value.status == COMPLETE){
            state_ = PARSE_LF_AND_SET; 
          }
          else{
            receiver_.receive_line(forward_buffer, false);
            return return_value;
          }
          break;

        case PARSE_LF_AND_SET:
          parse_char_result = parse_char(buffer, LF);
          receiver_.receive_line(forward_buffer, parse_char_result.first == COMPLETE);
          buffer = parse_char_result.second;
          if(parse_char_result.first == COMPLETE){
            state_ = PARSE_COMPLETE;
          }
          return_value.status = parse_char_result.first;
          return_value.remaining_buffer = buffer;
          return return_value;

        case PARSE_LF:
          parse_char_result = parse_char(buffer, LF);
          buffer = parse_char_result.second;
          receiver_.receive_line(asio::const_buffer(), true);
          if(parse_char_result.first == COMPLETE){
            state_ = PARSE_COMPLETE;
          }
          return_value.status = parse_char_result.first;
          return_value.remaining_buffer = buffer;
          return return_value;  
      }
    }
    CHECK_CONDITION_VAL(false, "Fell out of parse line loop", state_);
    return return_value; 
  }

}// namespace httplib
}// namespace switchflow
