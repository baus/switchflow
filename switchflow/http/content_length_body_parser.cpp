//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "content_length_body_parser.hpp"

#include <assert.h>

namespace http{
  
ContentLengthBodyParser::ContentLengthBodyParser(i_body_receiver* pBodyReceiver):
  pBodyReceiver_(pBodyReceiver)
{
  reset(0);
}

void ContentLengthBodyParser::reset(unsigned int messageSize)
{
  parseState_ = MESSAGE_BODY_PARSE;
  contentLength_ = messageSize;
  currentOffset_ = 0;
  currentLength_ = 0;
  
}

ContentLengthBodyParser::~ContentLengthBodyParser()
{
}

STATUS ContentLengthBodyParser::parseContentLengthBody(read_write_buffer& buffer)
{
  STATUS returnValue = INVALID;
  bool currentBufferProcessed = false;
  while(!currentBufferProcessed){
    switch(parseState_){
      case MESSAGE_BODY_PARSE:
        buffer.setWritePosition(buffer.getProcessPosition());

        returnValue = parseNLengthBuffer(buffer, currentLength_, contentLength_);

        buffer.setWriteEndPosition(buffer.getProcessPosition());

        if(returnValue == COMPLETE){
          transitionToState(COMPLETE_MESSAGE_BODY_FORWARD);
        }
        else if(returnValue == INCOMPLETE){
          transitionToState(INCOMPLETE_MESSAGE_BODY_FORWARD);
        }
        break;
      case INCOMPLETE_MESSAGE_BODY_FORWARD:
        returnValue = pBodyReceiver_->set_body(buffer, false);
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
        returnValue = pBodyReceiver_->set_body(buffer, true);
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
  parseState_ = newState;
}

}
