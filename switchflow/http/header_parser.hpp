//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All Rights Reserved
//
#ifndef SSD_HTTPHeaderParser_hpp
#define SSD_HTTPHeaderParser_hpp
#include <set>

#include <util/read_write_buffer.hpp>

#include "http.hpp" 

namespace http{

class IHeaderReceiver;  

class HTTPHeaderParser
{
 public:
  HTTPHeaderParser(IHeaderReceiver* pHeaderReceiver);

  enum PARSE_OPTION
  {
    STRICT,
    LOOSE
  };

  STATUS parseHeaders(read_write_buffer& buffer, PARSE_OPTION option=STRICT);
  
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

  STATUS parseStatusLine(read_write_buffer& buffer, PARSE_OPTION option);

  void nextStatusLineState(char delimiter);
  
  void transitionToState(STATUS_LINE_PARSE_STATE newState);
  
  void transitionToState(PARSE_STATE newState,
                         TRANSITION_CURRENT_LENGTH_OPTION
                           resetOption = RESET_CURRENT_LENGTH);


  STATUS parseStatusToken(read_write_buffer& buffer,
                          unsigned int beginOffset,
                          unsigned int& endOffset,
                          PARSE_OPTION option);

  STATUS receiveStatusLineToken(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);

  std::set<char>& getStatusTokenDelimiters(PARSE_OPTION option);

  // 
  // data members
  //
  PARSE_STATE currentState_;
  
  STATUS_LINE_PARSE_STATE statusLineState_;
  
  unsigned int currentLength_;
    
  IHeaderReceiver* pHeaderReceiver_;
  read_write_buffer chunkSizeBuffer_;
  int messageSize_;
};

} //namespace httplib

#endif

