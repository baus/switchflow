//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <assert.h>

#include <util/conversions.h>
#include <util/logger.hpp>

#include "chunked_body_parser.hpp"
#include "i_body_receiver.hpp"

namespace http{

ChunkedBodyParser::ChunkedBodyParser(i_body_receiver* pBodyReceiver, unsigned int maxChunksizeLength):
  pBodyReceiver_(pBodyReceiver),
  state_(PARSE_CHUNKSIZE),
  chunksize_(0),
  currentChunksizeLength_(0),
  maxChunksizeLength_(maxChunksizeLength),
  currentChunkLength_(0),
  numChunkSizeSpaces_(0)
{
  
}


ChunkedBodyParser::~ChunkedBodyParser()
{
}

void ChunkedBodyParser::reset()
{
  state_ = PARSE_CHUNKSIZE;
  chunksize_ = 0;
  currentChunksizeLength_ = 0;
  currentChunkLength_ = 0;
  numChunkSizeSpaces_ = 0;
}

STATUS ChunkedBodyParser::parseBody(read_write_buffer& buffer)
{
  STATUS status = INVALID;
  for(;;){
    switch(state_){
      case PARSE_CHUNKSIZE:
        status = parseChunkSize(buffer);
        buffer.setWritePosition(buffer.getProcessPosition());
        buffer.setWriteEndPosition(buffer.getProcessPosition());
        if(status == COMPLETE){
          state_ = PARSE_CHUNKSIZE_LF;
          break;
        }        
        return status;

      case PARSE_CHUNKSIZE_LF:
        status = parseChar(buffer, LF);
        if(status == COMPLETE){
          pBodyReceiver_->set_chunk_size(chunksize_);
          state_ = FORWARD_CHUNKSIZE;
          break;
        }
        return status;
        
      case FORWARD_CHUNKSIZE:
        status = pBodyReceiver_->forward_chunk_size();
        if(status == COMPLETE){
          if(chunksize_ > 0){
            state_ = PARSE_CHUNK;
            break;
          }
          else{
            buffer.setWritePosition(buffer.getProcessPosition());
            state_ = PARSE_TRAILER_CR;
            break;
          }  
        }
        if(status == INCOMPLETE){
          return WRITE_INCOMPLETE;
        }
        return status;
        
      case PARSE_CHUNK:
        buffer.setWritePosition(buffer.getProcessPosition());
        status = parseNLengthBuffer(buffer, currentChunkLength_, chunksize_);
        buffer.setWriteEndPosition(buffer.getProcessPosition());
        if(status == COMPLETE){
          state_ = FORWARD_COMPLETE_CHUNK;
        }
        if(status == INCOMPLETE){
          state_ = FORWARD_INCOMPLETE_CHUNK;
        }
        break;
        
      case FORWARD_INCOMPLETE_CHUNK:
        status = pBodyReceiver_->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.setWriteEndPosition(buffer.getWorkingLength());
          status = INCOMPLETE;
          state_ = PARSE_CHUNK;
        }
        else if(status == INCOMPLETE){
          status = WRITE_INCOMPLETE;
        }
        // always return from this state, since there is no more data to read.
        return status;
        
      case FORWARD_COMPLETE_CHUNK:
        status = pBodyReceiver_->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.setWriteEndPosition(buffer.getWorkingLength());
          state_ = PARSE_TRAILER_CR;
          break;
        }
        if(status == INCOMPLETE){
          status = WRITE_INCOMPLETE;
        }
        return status;
              
      case PARSE_TRAILER_CR:
        status = parseChar(buffer, CR);

        if(status == COMPLETE){
          buffer.setWritePosition(buffer.getProcessPosition());
          state_ = PARSE_TRAILER_LF;
          break;
        }
        return status;
        
      case PARSE_TRAILER_LF:
        status = parseChar(buffer, LF);
        
        if(status == COMPLETE){
          // parse next chunk
          buffer.setWritePosition(buffer.getProcessPosition());
          state_ = FORWARD_TRAILER_CRLF;
          break;
        }
        return status;
        
      case FORWARD_TRAILER_CRLF:
        status = pBodyReceiver_->forward_chunk_trailer();
        if(status == COMPLETE){
          if(chunksize_ == 0){
            read_write_buffer null_buffer(0u);
            status = pBodyReceiver_->set_body(null_buffer, true);
            return COMPLETE;
          }
          state_ = PARSE_CHUNKSIZE;
          reset();
          break;
        }
        if(status == INCOMPLETE){
          return WRITE_INCOMPLETE;
        }
        return status;
        
      default:
        CHECK_CONDITION_VAL(false, "invalid state in ChunkedBodyParser", state_);
        break;
    }  
  }
  CHECK_CONDITION(false, "Chunked Body Parser fell out of loop.");
  return INVALID;
}

STATUS ChunkedBodyParser::parseChunkSize(read_write_buffer& buffer)
{
  for(;;){
    if(buffer.getProcessPosition() >= buffer.getWorkingLength()){
      return INCOMPLETE;
    }
    BYTE ch = buffer[buffer.getProcessPosition()];
    if(ISHEX(ch) && !numChunkSizeSpaces_){
      if(currentChunksizeLength_ >= maxChunksizeLength_){
        numChunkSizeSpaces_ = 0;
        return DATAOVERFLOW;
      }
      chunksize_ = 16 * chunksize_ + HC2INT(ch);
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      ++currentChunksizeLength_;
    }
    else if(ch == ' ' && numChunkSizeSpaces_ < 256){
      //
      // This is a hack to allow a trailing space in chunksizes.
      // Apache 1.3 appears to incorrectly do this.
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      ++numChunkSizeSpaces_;
    }
    else if(ch == CR){
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      numChunkSizeSpaces_ = 0;
      return COMPLETE;
    }
    else{
      numChunkSizeSpaces_ = 0;
      return INVALID;
    }
  }
  CHECK_CONDITION(false, "fell out of chunk size loop");
  return INVALID;
}

} //namespace httplib
