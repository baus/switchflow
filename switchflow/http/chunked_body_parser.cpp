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
  m_pBodyReceiver(pBodyReceiver),
  m_state(PARSE_CHUNKSIZE),
  m_chunksize(0),
  m_currentChunksizeLength(0),
  m_maxChunksizeLength(maxChunksizeLength),
  m_currentChunkLength(0),
  m_numChunkSizeSpaces(0)
{
  
}


ChunkedBodyParser::~ChunkedBodyParser()
{
}

void ChunkedBodyParser::reset()
{
  m_state = PARSE_CHUNKSIZE;
  m_chunksize = 0;
  m_currentChunksizeLength = 0;
  m_currentChunkLength = 0;
  m_numChunkSizeSpaces = 0;
}

STATUS ChunkedBodyParser::parseBody(read_write_buffer& buffer)
{
  STATUS status = INVALID;
  for(;;){
    switch(m_state){
      case PARSE_CHUNKSIZE:
        status = parseChunkSize(buffer);
        buffer.setWritePosition(buffer.getProcessPosition());
        buffer.setWriteEndPosition(buffer.getProcessPosition());
        if(status == COMPLETE){
          m_state = PARSE_CHUNKSIZE_LF;
          break;
        }        
        return status;

      case PARSE_CHUNKSIZE_LF:
        status = parseChar(buffer, LF);
        if(status == COMPLETE){
          m_pBodyReceiver->set_chunk_size(m_chunksize);
          m_state = FORWARD_CHUNKSIZE;
          break;
        }
        return status;
        
      case FORWARD_CHUNKSIZE:
        status = m_pBodyReceiver->forward_chunk_size();
        if(status == COMPLETE){
          if(m_chunksize > 0){
            m_state = PARSE_CHUNK;
            break;
          }
          else{
            buffer.setWritePosition(buffer.getProcessPosition());
            m_state = PARSE_TRAILER_CR;
            break;
          }  
        }
        if(status == INCOMPLETE){
          return WRITE_INCOMPLETE;
        }
        return status;
        
      case PARSE_CHUNK:
        buffer.setWritePosition(buffer.getProcessPosition());
        status = parseNLengthBuffer(buffer, m_currentChunkLength, m_chunksize);
        buffer.setWriteEndPosition(buffer.getProcessPosition());
        if(status == COMPLETE){
          m_state = FORWARD_COMPLETE_CHUNK;
        }
        if(status == INCOMPLETE){
          m_state = FORWARD_INCOMPLETE_CHUNK;
        }
        break;
        
      case FORWARD_INCOMPLETE_CHUNK:
        status = m_pBodyReceiver->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.setWriteEndPosition(buffer.getWorkingLength());
          status = INCOMPLETE;
          m_state = PARSE_CHUNK;
        }
        else if(status == INCOMPLETE){
          status = WRITE_INCOMPLETE;
        }
        // always return from this state, since there is no more data to read.
        return status;
        
      case FORWARD_COMPLETE_CHUNK:
        status = m_pBodyReceiver->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.setWriteEndPosition(buffer.getWorkingLength());
          m_state = PARSE_TRAILER_CR;
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
          m_state = PARSE_TRAILER_LF;
          break;
        }
        return status;
        
      case PARSE_TRAILER_LF:
        status = parseChar(buffer, LF);
        
        if(status == COMPLETE){
          // parse next chunk
          buffer.setWritePosition(buffer.getProcessPosition());
          m_state = FORWARD_TRAILER_CRLF;
          break;
        }
        return status;
        
      case FORWARD_TRAILER_CRLF:
        status = m_pBodyReceiver->forward_chunk_trailer();
        if(status == COMPLETE){
          if(m_chunksize == 0){
            read_write_buffer null_buffer(0u);
            status = m_pBodyReceiver->set_body(null_buffer, true);
            return COMPLETE;
          }
          m_state = PARSE_CHUNKSIZE;
          reset();
          break;
        }
        if(status == INCOMPLETE){
          return WRITE_INCOMPLETE;
        }
        return status;
        
      default:
        CHECK_CONDITION_VAL(false, "invalid state in ChunkedBodyParser", m_state);
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
    if(ISHEX(ch) && !m_numChunkSizeSpaces){
      if(m_currentChunksizeLength >= m_maxChunksizeLength){
        m_numChunkSizeSpaces = 0;
        return DATAOVERFLOW;
      }
      m_chunksize = 16 * m_chunksize + HC2INT(ch);
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      ++m_currentChunksizeLength;
    }
    else if(ch == ' ' && m_numChunkSizeSpaces < 256){
      //
      // This is a hack to allow a trailing space in chunksizes.
      // Apache 1.3 appears to incorrectly do this.
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      ++m_numChunkSizeSpaces;
    }
    else if(ch == CR){
      buffer.setProcessPosition(buffer.getProcessPosition() + 1);
      m_numChunkSizeSpaces = 0;
      return COMPLETE;
    }
    else{
      m_numChunkSizeSpaces = 0;
      return INVALID;
    }
  }
  CHECK_CONDITION(false, "fell out of chunk size loop");
  return INVALID;
}

} //namespace httplib
