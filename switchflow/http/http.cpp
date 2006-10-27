//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
#include <assert.h>

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

std::set<char> s_spaceDelimiters;
std::set<char> s_separators;
std::set<char> s_fieldDelimiters;
std::set<char> s_endlineDelimiters;
std::set<char> s_lfDelimiters;
std::set<char> s_whiteSpace;
std::set<char> s_spaceEndlineDelimiters;

void init()
{
  s_whiteSpace.insert(HT);
  s_whiteSpace.insert(SP);

  s_spaceDelimiters.insert(SP);

  s_fieldDelimiters.insert(':');

  s_endlineDelimiters.insert(CR);

  s_lfDelimiters.insert(LF);

  s_spaceEndlineDelimiters.insert(SP);
  s_spaceEndlineDelimiters.insert(CR);
  
  
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

STATUS parseChar(read_write_buffer& buffer, char c)
{
  if(buffer.getProcessPosition() >= static_cast<unsigned int>(buffer.getWorkingLength())){
    return INCOMPLETE;
  }
  if(buffer[buffer.getProcessPosition()] == c){
    buffer.setProcessPosition(buffer.getProcessPosition() + 1);
    return COMPLETE;
  }
  return INVALID;
}

//
// Should the endOffset include the delimiters?  It currently does.
//
STATUS parseToken(read_write_buffer& buffer,
                  unsigned int beginOffset,
                  unsigned int& endOffset,
                  unsigned int& currentLength,
                  unsigned int maxLength,
                  std::set<char>& delimiters)
{
  int bufferSize = buffer.getWorkingLength();
  int i;
  for(i = beginOffset; i < bufferSize && currentLength < maxLength; ++i, ++currentLength){
    if(delimiters.find(buffer[i]) != delimiters.end()){
      //
      // found delimiter
      //
      endOffset = i + 1;
      return COMPLETE;
    }
    //
    // This is correct for Tokens, but I am using this function to parse URIs.  That
    // needs to be fixed...
    //    if(s_separators.find(buffer[i]) != s_separators.end() || isCTLChar(buffer[i])){
    if(isCTLChar(buffer[i])){
      //
      // found separator other than than those used to delimit
      // the token, or a control character.  Either is Invaild
      //
      log_info("invalid delimiter in HTTP token");
      return INVALID;
    }
  }
  if(currentLength >= maxLength){
    log_info("HTTP token exceeded defined size");
    return DATAOVERFLOW;
  }
  endOffset = i;
  return INCOMPLETE;
}

STATUS parseChar(read_write_buffer& buffer,
                 unsigned int beginOffset,
                 unsigned int& endOffset,
                 char c)
{
  if(beginOffset >= static_cast<unsigned int>(buffer.getWorkingLength())){
    endOffset = beginOffset;
    return INCOMPLETE;
  }
  if(buffer[beginOffset] == c){
    endOffset = beginOffset + 1;
    return COMPLETE;
  }
  return INVALID;
}


bool isCTLChar(char c)
{
  return c <= 31 || c == DEL;
}


STATUS parseEqual(read_write_buffer& buffer,
                  unsigned int beginOffset,
                  unsigned int& endOffset,
                  unsigned int& currentLength,
                  unsigned int maxLength,
                  std::set<char>& compareChars)
{
  int bufferSize = buffer.getWorkingLength();
  int i;
  for(i = beginOffset; i < bufferSize && currentLength < maxLength; ++i, ++currentLength){
    if(s_spaceDelimiters.find(buffer[i]) == s_spaceDelimiters.end()){
      //
      // not equal break
      //
      endOffset = i;
      return COMPLETE;
    }
  }
  if(currentLength >= maxLength){
    return DATAOVERFLOW;
  }
  endOffset = i;
  return INCOMPLETE;
}

STATUS parseNLengthBuffer(read_write_buffer& buffer, 
                          unsigned int& currentLength, 
                          unsigned int maxLength)
{
  unsigned int bufferSize = buffer.getWorkingLength();
  unsigned int beginOffset = buffer.getProcessPosition();

  unsigned int charsLeftInBuffer = bufferSize - beginOffset;
  unsigned int charsToAdd = charsLeftInBuffer < (maxLength - currentLength)?charsLeftInBuffer:(maxLength - currentLength);
  currentLength += charsToAdd;
  assert(currentLength <= maxLength);
  buffer.setProcessPosition(beginOffset + charsToAdd);
  return (currentLength < maxLength)?INCOMPLETE:COMPLETE;
}

void line_parser::reset(LINE_END_OPTION option, unsigned int max_length)
{
  line_end_option_ = option;
  current_length_ = 0;
  max_length_ = max_length;
  state_ = PARSE_LINE;
}

STATUS line_parser::parse_line(read_write_buffer& buffer,
                               unsigned int beginOffset,
                               unsigned int& endOffset)
{
  STATUS parse_status;
  while(true){
    if(state_ == PARSE_COMPLETE){
      return INVALID;
    }
    if(state_ == PARSE_LINE){
      parse_status = parseToken(buffer,
                                beginOffset,
                                endOffset,
                                current_length_,
                                max_length_,
                                s_endlineDelimiters);
      switch(parse_status){
        case COMPLETE:
          beginOffset = endOffset;
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
          CHECK_CONDITION_VAL(false, "invalid parse_line parseToken state", parse_status);
      };
    }
    else if(state_ == PARSE_LF){
      parse_status = parseChar(buffer,
                               beginOffset,
                               endOffset,
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
          CHECK_CONDITION_VAL(false, "invalid parse_line parseChar state", parse_status);
       };
    }
  }
}





}; // namespace httplib
