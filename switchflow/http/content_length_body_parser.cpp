//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "content_length_body_parser.hpp"

#include <assert.h>

namespace http{
  
ContentLengthBodyParser::ContentLengthBodyParser(i_body_receiver* pBodyReceiver):
  m_pBodyReceiver(pBodyReceiver)
{
  reset(0);
}

void ContentLengthBodyParser::reset(unsigned int messageSize)
{
  m_parseState = MESSAGE_BODY_PARSE;
  m_contentLength = messageSize;
  m_currentOffset = 0;
  m_currentLength = 0;
  
}

ContentLengthBodyParser::~ContentLengthBodyParser()
{
}

STATUS ContentLengthBodyParser::parseContentLengthBody(read_write_buffer& buffer)
{
  STATUS returnValue = INVALID;
  bool currentBufferProcessed = false;
  while(!currentBufferProcessed){
    switch(m_parseState){
      case MESSAGE_BODY_PARSE:
        buffer.setWritePosition(buffer.getProcessPosition());

        returnValue = parseNLengthBuffer(buffer, m_currentLength, m_contentLength);

        buffer.setWriteEndPosition(buffer.getProcessPosition());

        if(returnValue == COMPLETE){
          transitionToState(COMPLETE_MESSAGE_BODY_FORWARD);
        }
        else if(returnValue == INCOMPLETE){
          transitionToState(INCOMPLETE_MESSAGE_BODY_FORWARD);
        }
        break;
      case INCOMPLETE_MESSAGE_BODY_FORWARD:
        returnValue = m_pBodyReceiver->set_body(buffer, false);
        if(returnValue == COMPLETE){
          transitionToState(MESSAGE_BODY_PARSE);
          returnValue = INCOMPLETE;
        }
        else if(returnValue == INCOMPLETE){
          returnValue = WRITE_INCOMPLETE;
        }
        currentBufferProcessed = true;
        
        break;
      case COMPLETE_MESSAGE_BODY_FORWARD:
        returnValue = m_pBodyReceiver->set_body(buffer, true);
        if(returnValue == INCOMPLETE){
          returnValue = WRITE_INCOMPLETE;
        }
        currentBufferProcessed = true;
        break;
      default:
        CHECK_CONDITION(false, "Invalid State while parsing HTTP body.");
        break;
    }
  }
  return returnValue;
}

void ContentLengthBodyParser::transitionToState(ContentLengthBodyParser::PARSE_STATE newState)
{
  m_parseState = newState;
}

}
