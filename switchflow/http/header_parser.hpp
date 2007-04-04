//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_HTTP_HEADER_PARSER_HPP
#define SSD_HTTP_HEADER_PARSER_HPP
#include <set>

#include <util/read_write_buffer.hpp>

#include "http.hpp" 

namespace http{

class i_header_receiver;  

class http_header_parser
{
 public:
  http_header_parser(i_header_receiver* p_header_receiver);

  enum PARSE_OPTION
  {
    STRICT,
    LOOSE
  };

  STATUS parse_headers(read_write_buffer& buffer, PARSE_OPTION option=STRICT);
  
  void reset();


 private:

    
  enum STATUS_LINE_PARSE_STATE
  {
    STATUS_LINE_TOKEN_1,            // 0
    STATUS_LINE_TOKEN_2,            // 1
    STATUS_LINE_TOKEN_3,            // 2
    STATUS_LINE_LF                 // 3
  };    

  enum PARSE_STATE
  {
    STATUS_LINE,                    // 0 
    END_HEADERS_CR,                 // 1
    END_HEADERS_LF,                 // 2
    FIELD_NAME,                     // 3
    FIELD_VALUE_LEADING_LWS,        // 4
    FIELD_VALUE_LWS_LF,             // 5
    FIELD_VALUE_LWS,                // 6
    FIELD_VALUE_CONTENT             // 7
  };

  enum TRANSITION_CURRENT_LENGTH_OPTION
  {
    RESET_CURRENT_LENGTH,
    NOT_RESET_CURRENT_LENGTH
  };

  STATUS parse_status_line(read_write_buffer& buffer, PARSE_OPTION option);

  void next_status_line_state(char delimiter);
  
  void transition_to_state(STATUS_LINE_PARSE_STATE new_state);
  
  void transition_to_state(PARSE_STATE new_state,
                         TRANSITION_CURRENT_LENGTH_OPTION
                           reset_option = RESET_CURRENT_LENGTH);


  STATUS parse_status_token(read_write_buffer& buffer,
                          unsigned int begin_offset,
                          unsigned int& end_offset,
                          PARSE_OPTION option);

  STATUS receive_status_line_token(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete);

  std::set<char>& get_status_token_delimiters(PARSE_OPTION option);

  // 
  // data members
  //
  PARSE_STATE current_state_;
  
  STATUS_LINE_PARSE_STATE status_line_state_;
  
  unsigned int current_length_;
    
  i_header_receiver* p_header_receiver_;
  read_write_buffer chunk_size_buffer_;
  int message_size_;
};

} //namespace httplib

#endif

