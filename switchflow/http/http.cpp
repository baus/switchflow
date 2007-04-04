//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/logger.hpp>

#include "http.hpp"

namespace http{
  
const int CR = 13;
const int LF = 10;
const int SP = 32;
const int HT = 9;
const int DEL = 127;

// A chunksize length of 8 means 4 bytes or 32bit chunksize. 
//  
const unsigned int CHUNK_SIZE_LENGTH = 8;

const unsigned int MAX_FIELD_LENGTH = 512;

std::set<char> s_space_delimiters;
std::set<char> s_separators;
std::set<char> s_field_delimiters;
std::set<char> s_endline_delimiters;
std::set<char> s_lf_delimiters;
std::set<char> s_white_space;
std::set<char> s_space_endline_delimiters;

void init()
{
  s_white_space.insert(HT);
  s_white_space.insert(SP);

  s_space_delimiters.insert(SP);

  s_field_delimiters.insert(':');

  s_endline_delimiters.insert(CR);

  s_lf_delimiters.insert(LF);

  s_space_endline_delimiters.insert(SP);
  s_space_endline_delimiters.insert(CR);
  
  
  s_separators.insert('(');
  s_separators.insert(')');
  s_separators.insert('<');
  s_separators.insert('>');
  s_separators.insert('@');
  s_separators.insert(',');
  s_separators.insert(';');
  s_separators.insert(':');
  s_separators.insert('\\');
  s_separators.insert('"');
  s_separators.insert('/');
  s_separators.insert('[');
  s_separators.insert(']');
  s_separators.insert('?');
  s_separators.insert('=');
  s_separators.insert('{');
  s_separators.insert('}');
  s_separators.insert(SP);
  s_separators.insert(HT);
}

STATUS parse_char(read_write_buffer& buffer, char c)
{
  if(buffer.get_process_position() >= static_cast<unsigned int>(buffer.get_working_length())){
    return INCOMPLETE;
  }
  if(buffer[buffer.get_process_position()] == c){
    buffer.set_process_position(buffer.get_process_position() + 1);
    return COMPLETE;
  }
  return INVALID;
}

//
// Should the end_offset include the delimiters?  It currently does.
//
STATUS parse_token(read_write_buffer& buffer,
                  unsigned int begin_offset,
                  unsigned int& end_offset,
                  unsigned int& current_length,
                  unsigned int max_length,
                  std::set<char>& delimiters)
{
  int buffer_size = buffer.get_working_length();
  int i;
  for(i = begin_offset; i < buffer_size && current_length < max_length; ++i, ++current_length){
    if(delimiters.find(buffer[i]) != delimiters.end()){
      //
      // found delimiter
      //
      end_offset = i + 1;
      return COMPLETE;
    }
    //
    // This is correct for Tokens, but I am using this function to parse URIs.  That
    // needs to be fixed...
    //    if(s_separators.find(buffer[i]) != s_separators.end() || is_ctl_char(buffer[i])){
    if(is_ctl_char(buffer[i])){
      //
      // found separator other than than those used to delimit
      // the token, or a control character.  Either is Invaild
      //
      log_info("invalid delimiter in HTTP token");
      return INVALID;
    }
  }
  if(current_length >= max_length){
    log_info("HTTP token exceeded defined size");
    return DATAOVERFLOW;
  }
  end_offset = i;
  return INCOMPLETE;
}

STATUS parse_char(read_write_buffer& buffer,
                 unsigned int begin_offset,
                 unsigned int& end_offset,
                 char c)
{
  if(begin_offset >= static_cast<unsigned int>(buffer.get_working_length())){
    end_offset = begin_offset;
    return INCOMPLETE;
  }
  if(buffer[begin_offset] == c){
    end_offset = begin_offset + 1;
    return COMPLETE;
  }
  return INVALID;
}


bool is_ctl_char(char c)
{
  return c <= 31 || c == DEL;
}


STATUS parse_equal(read_write_buffer& buffer,
                  unsigned int begin_offset,
                  unsigned int& end_offset,
                  unsigned int& current_length,
                  unsigned int max_length,
                  std::set<char>& compare_chars)
{
  int buffer_size = buffer.get_working_length();
  int i;
  for(i = begin_offset; i < buffer_size && current_length < max_length; ++i, ++current_length){
    if(s_space_delimiters.find(buffer[i]) == s_space_delimiters.end()){
      //
      // not equal break
      //
      end_offset = i;
      return COMPLETE;
    }
  }
  if(current_length >= max_length){
    return DATAOVERFLOW;
  }
  end_offset = i;
  return INCOMPLETE;
}

STATUS parse_n_length_buffer(read_write_buffer& buffer, 
                          unsigned int& current_length, 
                          unsigned int max_length)
{
  unsigned int buffer_size = buffer.get_working_length();
  unsigned int begin_offset = buffer.get_process_position();

  unsigned int chars_left_in_buffer = buffer_size - begin_offset;
  unsigned int chars_to_add = chars_left_in_buffer < (max_length - current_length)?chars_left_in_buffer:(max_length - current_length);
  current_length += chars_to_add;
  CHECK_CONDITION(current_length <= max_length, "buffer longer than maximum allowed");
  buffer.set_process_position(begin_offset + chars_to_add);
  return (current_length < max_length)?INCOMPLETE:COMPLETE;
}

void line_parser::reset(LINE_END_OPTION option, unsigned int max_length)
{
  line_end_option_ = option;
  current_length_ = 0;
  max_length_ = max_length;
  state_ = PARSE_LINE;
}

STATUS line_parser::parse_line(read_write_buffer& buffer,
                               unsigned int begin_offset,
                               unsigned int& end_offset)
{
  STATUS parse_status;
  while(true){
    if(state_ == PARSE_COMPLETE){
      return INVALID;
    }
    if(state_ == PARSE_LINE){
      parse_status = parse_token(buffer,
                                begin_offset,
                                end_offset,
                                current_length_,
                                max_length_,
                                s_endline_delimiters);
      switch(parse_status){
        case COMPLETE:
          begin_offset = end_offset;
          state_ = PARSE_LF;
          break;
        case INVALID:
        case DATAOVERFLOW:
          state_ = PARSE_COMPLETE;
          return parse_status;
          break;
        case INCOMPLETE:
          return parse_status;
          break;
        default:
          CHECK_CONDITION_VAL(false, "invalid parse_line parse_token state", parse_status);
      };
    }
    else if(state_ == PARSE_LF){
      parse_status = parse_char(buffer,
                               begin_offset,
                               end_offset,
                               LF);
      switch(parse_status){
        case COMPLETE:
        case INVALID:
        case DATAOVERFLOW:
          state_ = PARSE_COMPLETE;
          return parse_status;
          break;
        case INCOMPLETE:
          return parse_status;
          break;
        default:
          CHECK_CONDITION_VAL(false, "invalid parse_line parse_char state", parse_status);
       };
    }
  }
}





}; // namespace httplib
