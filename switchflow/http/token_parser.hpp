//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HTTP_TOKEN_PARSER_HPP
#define SF_HTTP_TOKEN_PARSER_HPP

#include <set>
#include <asio/buffer.hpp>
#include "parser_types.hpp" 

namespace switchflow{
namespace http{


class token_parser
{
 public:
  token_parser(size_t max_length, const std::set<char>& delimiters):
    max_length_(max_length)
    , delimiters_(delimiters)
    , current_length_(0){}
       
  template<typename token_receiver>
  parse_result parse(asio::const_buffer buffer, token_receiver receiver)
  {
    parse_result result;
    int buffer_size = asio::buffer_size(buffer);
    
    const char* raw_buffer = asio::buffer_cast<const char*>(buffer);
    
    int i;
    for(i = 0; i < buffer_size && current_length_ < max_length_; ++i, ++current_length_){
      if(delimiters_.find(raw_buffer[i]) != delimiters_.end()){
        //
        // found delimiter
        result.buffer = buffer + (i + 1);

        // Use temporary here so the receiver can be created using boost::bind.  
        // See.  http://www.boost.org/libs/bind/bind.html#Limitations
        buffer_status buf_status = COMPLETE;
        result.status = convert_to_parse_status(receiver(asio::const_buffer(raw_buffer, i), raw_buffer[i], buf_status), COMPLETE);
        current_length_ = 0;
        return result;
      }
      //
      // This is correct for Tokens, but I am using this function to parse URIs.  That
      // needs to be fixed...
      //    if(s_separators.find(buffer[i]) != s_separators.end() || is_ctl_char(buffer[i])){
      if(is_ctl_char(raw_buffer[i])){
        result.status = parse::INVALID;
        return result;
      }
    }
    if(current_length_ >= max_length_){
        result.status = parse::DATAOVERFLOW;
      return result;
    }

    result.buffer = buffer + i;

    // Use temporary here so the receiver can be created using boost::bind.  
    // See.  http://www.boost.org/libs/bind/bind.html#Limitations
    buffer_status buf_status = INCOMPLETE;
    result.status = convert_to_parse_status(receiver(asio::const_buffer(raw_buffer, i), raw_buffer[i], buf_status), INCOMPLETE);
  
    return result;  
  }
  
private:
  bool is_ctl_char(char c)
  {
    return c <= 31 || c == 127;  // 127 is the DEL char
  }

  size_t max_length_;
  const std::set<char>& delimiters_;
  size_t current_length_;
  
};


} //namespace httplib
} //namespace switchflow

#endif

