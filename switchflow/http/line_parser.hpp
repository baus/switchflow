//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HTTP_LINE_PARSER_HPP
#define SF_HTTP_LINE_PARSER_HPP

#include <boost/asio/buffer.hpp>
#include <boost/bind.hpp>

#include "token_parser.hpp"

namespace switchflow{
namespace http{


class line_parser
{
 public:
     line_parser(size_t max_length, const std::set<char>& delimiters):
       t_parser_(max_length, delimiters)
           , state_(PARSE_LINE)
           , send_complete_line_(false)
     {
        
     }
  
  template<typename line_receiver>
  receive::status token_receiver(boost::asio::const_buffer buffer, char delimiter, buffer_status buf_status, line_receiver receiver)
  {
    if(buf_status == BUFFER_COMPLETE){
        //
        // Store the line here to look for the LF w/out calling back
        complete_line_ = buffer;
        send_complete_line_ = true;
        state_ = PARSE_LF;
        return receive::SUCCESS;
    }
    else{
        // Use temporary here so the receiver can be created using boost::bind.  
        // See.  http://www.boost.org/libs/bind/bind.html#Limitations
        buffer_status buf_status = INCOMPLETE;
    }
    return receiver(buffer, buf_status);
  }

  template<typename line_receiver>
  parse_result parse(boost::asio::const_buffer buffer, line_receiver receiver)
  {
      while(true){
        if(state_ == PARSE_LINE){
          //
          // This bind syntax is uglier than hell.
          parse_result result = 
          t_parser_.parse(buffer, 
                          boost::bind(&line_parser::token_receiver<line_receiver>
                                      , this
                                      , _1
                                      , _2
                                      , _3
                                      , receiver));
          buffer = result.buffer;
        
          if(result.status != COMPLETE){
            return result;
          }
        }
        else if(state_ == PARSE_LF){
          if(boost::asio::buffer_size(buffer) < 1){
            parse_result result;
            
            if(send_complete_line_){
              buffer_status buf_status = INCOMPLETE;

              result.status = convert_to_parse_status(receiver(complete_line_, buf_status), INCOMPLETE);
              send_complete_line_ = false;
            }
            else{
              result.status = parse::INCOMPLETE;
            }
            result.buffer = buffer;
            return result;
          }
          char LF = 10;
          if(boost::asio::buffer_cast<const char*>(buffer)[0] == LF){
            parse_result result;
            if(send_complete_line_){
              buffer_status buf_status = COMPLETE;
              result.status = convert_to_parse_status(receiver(complete_line_, buf_status), COMPLETE);
              send_complete_line_ = false;
            }
            else{
              //
              // This is the strange case where the LF comes in a different buffer than the
              // CR.  In this case we've sent the data with an incomplete.  
              buffer_status buf_status = COMPLETE;
	      boost::asio::const_buffer empty_buffer;
              result.status = convert_to_parse_status(receiver(empty_buffer, buf_status), COMPLETE);
            }
            
            result.buffer = buffer + 1;
            state_ = PARSE_LINE;
            return result;
          }
          else{ 
            parse_result result;
            result.status = parse::INVALID;
            result.buffer = buffer;
            return result;
          }
        }
      }
      parse_result result;
      return result;
  }

  
private:
  enum STATE{
    PARSE_LINE,
    PARSE_LF
  };

  token_parser t_parser_;
  STATE state_;
  
  boost::asio::const_buffer complete_line_;
  bool send_complete_line_;

};

} //namespace httplib
} //namespace switchflow

#endif

