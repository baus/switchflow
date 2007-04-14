//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_LINE_PARSER_HPP
#define SF_LINE_PARSER_HPP

#include <set>
#include <asio.hpp>
#include "http.hpp" 

namespace switchflow{
namespace http{

class i_line_receiver;  

class line_parser
{
 public:
  line_parser(i_line_receiver& receiver, size_t max_length);
  
  
  parse_result parse_line(asio::const_buffer buffer);

  void reset();
  
private:
  enum STATE{
    PARSE_LINE,
    PARSE_LF_AND_SET,
    PARSE_LF,
    PARSE_COMPLETE
  };

  i_line_receiver& receiver_;
  size_t current_length_;
  size_t max_length_;
  STATE state_;
};

} //namespace httplib
} //namespace switchflow

#endif

