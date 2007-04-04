//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/conversions.hpp>
#include <util/logger.hpp>

#include "chunked_body_parser.hpp"
#include "i_body_receiver.hpp"

namespace http{

chunked_body_parser::chunked_body_parser(i_body_receiver* p_body_receiver, unsigned int max_chunksize_length):
  p_body_receiver_(p_body_receiver),
  state_(PARSE_CHUNKSIZE),
  chunksize_(0),
  current_chunksize_length_(0),
  max_chunksize_length_(max_chunksize_length),
  current_chunk_length_(0),
  num_chunk_size_spaces_(0)
{
  
}


chunked_body_parser::~chunked_body_parser()
{
}

void chunked_body_parser::reset()
{
  state_ = PARSE_CHUNKSIZE;
  chunksize_ = 0;
  current_chunksize_length_ = 0;
  current_chunk_length_ = 0;
  num_chunk_size_spaces_ = 0;
}

STATUS chunked_body_parser::parse_body(read_write_buffer& buffer)
{
  STATUS status = INVALID;
  for(;;){
    switch(state_){
      case PARSE_CHUNKSIZE:
        status = parse_chunk_size(buffer);
        buffer.set_write_position(buffer.get_process_position());
        buffer.set_write_end_position(buffer.get_process_position());
        if(status == COMPLETE){
          state_ = PARSE_CHUNKSIZE_LF;
          break;
        }        
        return status;

      case PARSE_CHUNKSIZE_LF:
        status = parse_char(buffer, LF);
        if(status == COMPLETE){
          p_body_receiver_->set_chunk_size(chunksize_);
          state_ = FORWARD_CHUNKSIZE;
          break;
        }
        return status;
        
      case FORWARD_CHUNKSIZE:
        status = p_body_receiver_->forward_chunk_size();
        if(status == COMPLETE){
          if(chunksize_ > 0){
            state_ = PARSE_CHUNK;
            break;
          }
          else{
            buffer.set_write_position(buffer.get_process_position());
            state_ = PARSE_TRAILER_CR;
            break;
          }  
        }
        if(status == INCOMPLETE){
          return WRITE_INCOMPLETE;
        }
        return status;
        
      case PARSE_CHUNK:
        buffer.set_write_position(buffer.get_process_position());
        status = parse_n_length_buffer(buffer, current_chunk_length_, chunksize_);
        buffer.set_write_end_position(buffer.get_process_position());
        if(status == COMPLETE){
          state_ = FORWARD_COMPLETE_CHUNK;
        }
        if(status == INCOMPLETE){
          state_ = FORWARD_INCOMPLETE_CHUNK;
        }
        break;
        
      case FORWARD_INCOMPLETE_CHUNK:
        status = p_body_receiver_->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.set_write_end_position(buffer.get_working_length());
          status = INCOMPLETE;
          state_ = PARSE_CHUNK;
        }
        else if(status == INCOMPLETE){
          status = WRITE_INCOMPLETE;
        }
        // always return from this state, since there is no more data to read.
        return status;
        
      case FORWARD_COMPLETE_CHUNK:
        status = p_body_receiver_->set_body(buffer, false);
        if(status == COMPLETE){
          buffer.set_write_end_position(buffer.get_working_length());
          state_ = PARSE_TRAILER_CR;
          break;
        }
        if(status == INCOMPLETE){
          status = WRITE_INCOMPLETE;
        }
        return status;
              
      case PARSE_TRAILER_CR:
        status = parse_char(buffer, CR);

        if(status == COMPLETE){
          buffer.set_write_position(buffer.get_process_position());
          state_ = PARSE_TRAILER_LF;
          break;
        }
        return status;
        
      case PARSE_TRAILER_LF:
        status = parse_char(buffer, LF);
        
        if(status == COMPLETE){
          // parse next chunk
          buffer.set_write_position(buffer.get_process_position());
          state_ = FORWARD_TRAILER_CRLF;
          break;
        }
        return status;
        
      case FORWARD_TRAILER_CRLF:
        status = p_body_receiver_->forward_chunk_trailer();
        if(status == COMPLETE){
          if(chunksize_ == 0){
            read_write_buffer null_buffer(0u);
            status = p_body_receiver_->set_body(null_buffer, true);
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
        CHECK_CONDITION_VAL(false, "invalid state in chunked_body_parser", state_);
        break;
    }  
  }
  CHECK_CONDITION(false, "chunked Body Parser fell out of loop.");
  return INVALID;
}

STATUS chunked_body_parser::parse_chunk_size(read_write_buffer& buffer)
{
  for(;;){
    if(buffer.get_process_position() >= buffer.get_working_length()){
      return INCOMPLETE;
    }
    BYTE ch = buffer[buffer.get_process_position()];
    if(ISHEX(ch) && !num_chunk_size_spaces_){
      if(current_chunksize_length_ >= max_chunksize_length_){
        num_chunk_size_spaces_ = 0;
        return DATAOVERFLOW;
      }
      chunksize_ = 16 * chunksize_ + HC2INT(ch);
      buffer.set_process_position(buffer.get_process_position() + 1);
      ++current_chunksize_length_;
    }
    else if(ch == ' ' && num_chunk_size_spaces_ < 256){
      //
      // This is a hack to allow a trailing space in chunksizes.
      // Apache 1.3 appears to incorrectly do this.
      buffer.set_process_position(buffer.get_process_position() + 1);
      ++num_chunk_size_spaces_;
    }
    else if(ch == CR){
      buffer.set_process_position(buffer.get_process_position() + 1);
      num_chunk_size_spaces_ = 0;
      return COMPLETE;
    }
    else{
      num_chunk_size_spaces_ = 0;
      return INVALID;
    }
  }
  CHECK_CONDITION(false, "fell out of chunk size loop");
  return INVALID;
}

} //namespace httplib
