//
// Copyright (c) Christopher Baus.  All Rights Reserved.
// 
#include <assert.h>

#include <algorithm>

#include <util/logger.hpp>

#include "http.hpp"
#include "header_parser.hpp"
#include "i_header_receiver.hpp"

namespace http{
  
HTTPHeaderParser::HTTPHeaderParser(IHeaderReceiver* pHeaderReceiver):
  m_currentState(STATUS_LINE),
  m_statusLineState(STATUS_LINE_TOKEN_1),
  m_currentLength(0),
  m_pHeaderReceiver(pHeaderReceiver),
  m_chunkSizeBuffer(10)
{
  
}

STATUS HTTPHeaderParser::parseStatusToken(read_write_buffer& buffer,
                                          unsigned int beginOffset,
                                          unsigned int& endOffset,
                                          PARSE_OPTION option)
{
  STATUS forwardStatus = COMPLETE;
  STATUS returnValue = parseToken(buffer,
                                  beginOffset,
                                  endOffset,
                                  m_currentLength,
                                  MAX_FIELD_LENGTH,
                                  getStatusTokenDelimiters(option));
  
  if(returnValue == COMPLETE){
    returnValue = receiveStatusLineToken(buffer, beginOffset, endOffset - 1, true);
    if(DATAOVERFLOW != returnValue && returnValue != INVALID){
      nextStatusLineState(buffer[endOffset - 1]);
    }
    else{
      log_info("Invalid status line token", m_statusLineState);
    }
  }
  else if(returnValue == INCOMPLETE){
    forwardStatus =  receiveStatusLineToken(buffer,
                                            beginOffset,
                                            endOffset,
                                            false);
    if(DATAOVERFLOW == forwardStatus || INVALID == forwardStatus){
      returnValue = forwardStatus;
    }   
  }
  else{
    log_info("Overflow status line token", m_statusLineState);
    returnValue = INVALID;
  }
  
  return returnValue;
  
}

STATUS HTTPHeaderParser::parseStatusLine(read_write_buffer& buffer, PARSE_OPTION option)
{
  
  //
  // Is this the correct initialization?
  //
  STATUS returnValue = COMPLETE;
  STATUS forwardStatus = COMPLETE;
  bool bufferParseComplete = false;
  unsigned int currentOffset = buffer.getProcessPosition();
  unsigned int beginOffset;
  bool finished = false;
  
  while(!bufferParseComplete){
    beginOffset = currentOffset;
    switch(m_statusLineState){
      case STATUS_LINE_TOKEN_1:
      case STATUS_LINE_TOKEN_2:
      case STATUS_LINE_TOKEN_3:
        returnValue = parseStatusToken(buffer, beginOffset, currentOffset, option);
        bufferParseComplete = returnValue != COMPLETE;
        break;
        
      case STATUS_LINE_LF:
        returnValue = parseChar(buffer, beginOffset, currentOffset, LF);
        bufferParseComplete = true;
        break;
    }
  }

  buffer.setProcessPosition(currentOffset);
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
  buffer.setWritePosition(currentOffset);

  return returnValue;
}

STATUS HTTPHeaderParser::parseHeaders(read_write_buffer& buffer, PARSE_OPTION option)
{

  //
  // Is this the correct initialization?
  //
  STATUS returnValue = COMPLETE;
  bool bufferParseComplete = false;
  unsigned int currentOffset = buffer.getProcessPosition();
  unsigned int beginOffset;
  bool finished = false;

  while(!bufferParseComplete){
    beginOffset = currentOffset;
    switch(m_currentState){
      case STATUS_LINE:
        returnValue = parseStatusLine(buffer, option);
        if(returnValue == COMPLETE){
          currentOffset = buffer.getProcessPosition();
          beginOffset = currentOffset;
          transitionToState(END_HEADERS_CR);
        }
        else{
          bufferParseComplete = true;
        }
        break;
        
      case END_HEADERS_CR:
        returnValue = parseChar(buffer, beginOffset, currentOffset, CR);
        if(returnValue == COMPLETE){
          transitionToState(END_HEADERS_LF);
        }
        else if(returnValue == INVALID){
          transitionToState(FIELD_NAME);
        }
        else{
          bufferParseComplete = true;
        }
        break;
      case END_HEADERS_LF:
        returnValue = parseChar(buffer, beginOffset, currentOffset, LF);
        bufferParseComplete = true;
        if(returnValue == COMPLETE){
          if(COMPLETE != (returnValue = m_pHeaderReceiver->endFields())){
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
        returnValue = parseToken(buffer, beginOffset, currentOffset, m_currentLength, MAX_FIELD_LENGTH, s_fieldDelimiters);
        if(returnValue == COMPLETE){
          if(DATAOVERFLOW == 
             m_pHeaderReceiver->setFieldName(buffer, beginOffset, currentOffset - 1, true)){
            bufferParseComplete = true;
            returnValue =  DATAOVERFLOW;
          }
          else{
            transitionToState(FIELD_VALUE_LEADING_LWS);
          }
        }
        else{
          if(returnValue == INCOMPLETE){
            if(DATAOVERFLOW == m_pHeaderReceiver->setFieldName(buffer, 
                                                               beginOffset,
                                                               currentOffset, 
                                                               false)){
              bufferParseComplete = true;
              returnValue = DATAOVERFLOW;
            }
          }
          bufferParseComplete = true;
        }
        break;
        
      case FIELD_VALUE_LEADING_LWS:
        returnValue = parseEqual(buffer, beginOffset, currentOffset, m_currentLength, MAX_FIELD_LENGTH, s_whiteSpace);
        if(returnValue == COMPLETE){
          transitionToState(FIELD_VALUE_CONTENT, NOT_RESET_CURRENT_LENGTH);
        }
        else{
          bufferParseComplete = true;
        }
        break;

      case FIELD_VALUE_LWS_LF:
        returnValue = parseChar(buffer, beginOffset, currentOffset, LF);
        if(returnValue == COMPLETE){
          transitionToState(FIELD_VALUE_LWS);
        }
        else{
          bufferParseComplete = true;
        }
        break;
      case FIELD_VALUE_LWS:
        returnValue = parseEqual(buffer, beginOffset, currentOffset, m_currentLength, MAX_FIELD_LENGTH, s_whiteSpace);
        if(returnValue == COMPLETE){
          if(currentOffset == beginOffset){
            //
            // empty, see if we are at the end of headers, then not get the next header..
            // but before we leave now is a good time to figure out how to parse the message
            // body...  This is the ugly, stateful part of HTTP parsing (ie the parsing of the
            // remainder of the message is determined by the message itself).  
            transitionToState(END_HEADERS_CR);
          }
          else{
            //
            // Looks like a line continuation, back to parsing the value
            //
            transitionToState(FIELD_VALUE_CONTENT);
          }
        }
        else{
          bufferParseComplete = true;
        }
        break;
      case FIELD_VALUE_CONTENT:
        returnValue = parseToken(buffer,
                                 beginOffset,
                                 currentOffset,
                                 m_currentLength,
                                 MAX_FIELD_LENGTH,
                                 s_endlineDelimiters);
      
        if(returnValue == COMPLETE){
          //
          // This doesn't deal with line continuations correctly.  I'm not sure if we are not looking a line
          // completion.  Sending true here isn't totally acurate.  Might get two trues back to back in the 
          // line continuation case...
          //
          if(DATAOVERFLOW == m_pHeaderReceiver->setFieldValue(buffer, beginOffset, currentOffset - 1, true)){
            bufferParseComplete = true;
            returnValue = DATAOVERFLOW;
          }
          transitionToState(FIELD_VALUE_LWS_LF);
        }
        else if(returnValue == INCOMPLETE){
          if(DATAOVERFLOW == m_pHeaderReceiver->setFieldValue(buffer, beginOffset, currentOffset, false)){
            bufferParseComplete = true;
            returnValue = DATAOVERFLOW;
          }
          else{
            bufferParseComplete = true;
          }
        }
        else{
          bufferParseComplete = true;
        }
        break;
      default:
        assert(false);
        break;
    }
  }
  buffer.setProcessPosition(currentOffset);
  //
  // Since we are not writting directly out of the buffer,
  // we can mark the buffer as written even though it really
  // is never written.  If we ever want to write headers directly
  // and not buffer then (highly unlikely), then this would be
  // incorrect.  The layer receiving the parsed buffer would
  // set the write flag.
  //
  // But for now this works as intended.
  //
  buffer.setWritePosition(currentOffset);
  if(returnValue == COMPLETE && !finished){
    return INCOMPLETE;
  }
  return returnValue;
}

void HTTPHeaderParser::nextStatusLineState(char delimiter)
{
  switch(m_statusLineState){
    case STATUS_LINE_TOKEN_1:
      CHECK_CONDITION_VAL(delimiter == SP, "invalid delimiter used to change status TOKEN_1 state", delimiter);
      m_statusLineState = STATUS_LINE_TOKEN_2;
      break;
    case STATUS_LINE_TOKEN_2:
      if(delimiter == CR){
        //
        // This is a pseudo - invalid status line that doesn't include the reason token.
        // Apache 1.3 generate status lines like this.  This should only happen when
        // loose parsing is allowed.
        m_statusLineState = STATUS_LINE_LF;
      }
      else if(delimiter == SP){
        m_statusLineState = STATUS_LINE_TOKEN_3;
      }
      else{
        CHECK_CONDITION_VAL(false, "Invalid delimiter use to change status TOKEN_2 state", delimiter);
      }
      break;
    case STATUS_LINE_TOKEN_3:
      CHECK_CONDITION_VAL(delimiter == CR, "Invalid delimiter use to change status TOKEN_3 state", delimiter);
      m_statusLineState = STATUS_LINE_LF;
      break;
    case STATUS_LINE_LF:
      m_statusLineState = STATUS_LINE_TOKEN_1;
      break;
    default:
      CHECK_CONDITION_VAL(false, "invalid start line state", m_statusLineState);
  }
  m_currentLength = 0;
}

void HTTPHeaderParser::transitionToState(PARSE_STATE newState,
                                         TRANSITION_CURRENT_LENGTH_OPTION
                                         resetOption)
{
  m_currentState = newState;
  if(resetOption == RESET_CURRENT_LENGTH){
    m_currentLength = 0;
  }
}

void HTTPHeaderParser::reset()
{
  m_statusLineState = STATUS_LINE_TOKEN_1;
  transitionToState(STATUS_LINE);
}

STATUS HTTPHeaderParser::receiveStatusLineToken(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete)
{
  STATUS returnValue;
  switch(m_statusLineState){
    case STATUS_LINE_TOKEN_1:
      returnValue = m_pHeaderReceiver->startLineToken1(buffer, iBegin, iEnd, bComplete);
      break;
    case STATUS_LINE_TOKEN_2:
      returnValue = m_pHeaderReceiver->startLineToken2(buffer, iBegin, iEnd, bComplete);
      break;
    case STATUS_LINE_TOKEN_3:
      returnValue = m_pHeaderReceiver->startLineToken3(buffer, iBegin, iEnd, bComplete);
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state to receieve status token", m_statusLineState);
      
  }

  return returnValue;
  
}
  
std::set<char>& HTTPHeaderParser::getStatusTokenDelimiters(PARSE_OPTION option)
{
  switch(m_statusLineState){
    case STATUS_LINE_TOKEN_1:
      return s_spaceDelimiters;
      break;
    case STATUS_LINE_TOKEN_2:
      if(option == STRICT){
        return s_spaceDelimiters;
      }
      else{
        return s_spaceEndlineDelimiters;
      }
      break;
    case STATUS_LINE_TOKEN_3:
      return s_endlineDelimiters;
      break;
    default:
      CHECK_CONDITION_VAL(false, "Invalid start line state to retrieve token delimiters", m_statusLineState);
  }

}

}// namespace httplib
