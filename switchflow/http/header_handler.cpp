//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <cstring>
#include <util/logger.hpp>

#include <http/http.hpp>
#include "request_buffer_wrapper.hpp"
#include "response_buffer_wrapper.hpp"
#include "header_handler.hpp"

namespace switchflow{
namespace http{

char * valid_methods[] = {"GET", "POST", "HEAD"};


//
// This information is static, but I might want to
// allow the user to configure this, so actually
// calculate the max_method_length.
size_t max_method_length()
{
  size_t max = 0;
  for(int i=0; i < NUM_VALID_METHODS; ++i)
  {
    size_t current_len = strlen(valid_methods[i]);
    // MAX seems to be totally screwed on GCC, so
    // just do it manually.
    if(current_len > max){
      max = current_len;
    } 
  }
  return max;
}

const unsigned int max_version_length = 8;


  header_handler::header_handler(message_buffer& response,
                                 STREAM_TYPE stream_type):
    message_buffer_(response),
    stream_type_(stream_type)
{
  reset();
}

header_handler::~header_handler()
{
  
}


void header_handler::reset()
{
  current_field_ = 0;
  have_host_header_ = false;
  ignore_body_encoding_headers_ = false;
  message_size_ = 0;
  message_buffer_.reset();
  if(stream_type_ == REQUEST){
    body_encoding_ = http::NONE;
  }
  else if(stream_type_ == RESPONSE){
    body_encoding_ = http::END_CONNECTION;
  }
  else{
    CHECK_CONDITION(false, "invalid stream type; should be REQUEST or RESPONSE");
  }
}

http::STATUS header_handler::start_line_token1(read_write_buffer& buffer, 
                                                int i_begin, 
                                                int i_end, 
                                                bool b_complete)
{
  if(message_buffer_.append_to_status_line_1(buffer.get_raw_buffer().begin() + i_begin,
                                        buffer.get_raw_buffer().begin() + i_end)){

    if(stream_type_ == REQUEST){
      for(int i = 0; i < NUM_VALID_METHODS; ++i){
        read_write_buffer::COMPARE_RESULT result = 
          message_buffer_.get_status_line_1().compare_no_case(valid_methods[i]);
        if(b_complete && result == read_write_buffer::EQUAL){
          return http::COMPLETE;
        }
        if(!b_complete && result == read_write_buffer::SUBSTRING){
          return http::COMPLETE;
        }
      }
      return http::INVALID;
    }
    else if(stream_type_ == RESPONSE){
      return http::COMPLETE;
    }
  }
  log_error("attempt to overrun buffer in HTTP startline 1");
  return http::DATAOVERFLOW;
}

http::STATUS header_handler::start_line_token2(read_write_buffer& buffer,
                                                int i_begin,
                                                int i_end,
                                                bool b_complete)
{
  if(message_buffer_.append_to_status_line_2(buffer.get_raw_buffer().begin() + i_begin, buffer.get_raw_buffer().begin() + i_end)){
    if(b_complete && stream_type_ == RESPONSE){
      scan_return_code();
    }
    return http::COMPLETE;
  }
  log_error("attempt to overrun buffer in HTTP startline 2");
  return http::DATAOVERFLOW;
}

http::STATUS header_handler::start_line_token3(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)
{


  if(message_buffer_.append_to_status_line_3(buffer.get_raw_buffer().begin() + i_begin, buffer.get_raw_buffer().begin() + i_end)){
    
    return http::COMPLETE;
  }
  log_error("attempt to overrun buffer in HTTP startline 3");
  return http::DATAOVERFLOW;
}

http::STATUS header_handler::set_field_name(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)
{
  if(message_buffer_.get_num_fields() == current_field_){
    if(current_field_ >= message_buffer_.get_max_fields()){
      log_error("attempt to exceed maximum number of header fields");
      return http::DATAOVERFLOW;
    }
    if(!message_buffer_.add_field()){
      log_error("header cache empty");
      return http::DATAOVERFLOW;
    }
  }

  if(message_buffer_.append_to_name(current_field_, buffer.get_raw_buffer().begin() + i_begin, buffer.get_raw_buffer().begin() + i_end)){
    return http::COMPLETE;
  }
  log_error("attempt to overrun buffer in header field name ");
  return http::DATAOVERFLOW;
}

http::STATUS header_handler::set_field_value(read_write_buffer& buffer, int i_begin, int i_end, bool b_complete)
{
  if(message_buffer_.append_to_value(current_field_, 
                                    buffer.get_raw_buffer().begin() + i_begin, 
                                    buffer.get_raw_buffer().begin() + i_end)){
    if(b_complete){
      scan_field();
      ++current_field_;
    }
    return http::COMPLETE;
  }
  log_error("attempt to overrun buffer in header field value ");
  return http::DATAOVERFLOW;
}

http::STATUS header_handler::end_fields()
{
  if(stream_type_ == REQUEST){
    request_buffer_wrapper request_wrapper(message_buffer_);
    if(request_wrapper.get_http_version() == request_buffer_wrapper::HTTP1_1){
      if(!have_host_header_){
        //
        // Must have host header, see HTTP 1.1 spec 14.23
        //
        log_error("HTTP 1.1 request didn't include header line.");
        return http::INVALID;
      }
    }
  }
  return http::COMPLETE;
}

void header_handler::scan_field()
{
  read_write_buffer& field_value = message_buffer_.get_field_value(current_field_);

  if(!have_host_header_ && message_buffer_.field_name_equals(current_field_, "host")){
    have_host_header_ = true;
  }
  //
  // This happens if the response is a 304
  if(ignore_body_encoding_headers_){
    return;
  }
  
  // Scanning all headers every time is inefficient.  If wasting cycles
  // at least look for duplicated headers and report error.
  if(message_buffer_.field_name_equals(current_field_, "content-length")){
    //
    // According to 4.4 of 1.1 spec we MUST ignore content length if a transfer-encoding
    // has been specified.
    //
    if(body_encoding_ != http::CHUNKED){
      body_encoding_ = http::CONTENT_LENGTH;
      *field_value.working_end() = 0;

      message_size_ = atoi((char*)&(field_value.get_raw_buffer()[0]));
    }
  }
  else if(message_buffer_.field_name_equals(current_field_, "transfer-encoding")){
    if(message_buffer_.field_value_equals(current_field_, "chunked")){
      body_encoding_ = http::CHUNKED;
    }
  }
}

void header_handler::scan_return_code()
{
  response_buffer_wrapper buffer_wrapper(message_buffer_);
  if(buffer_wrapper.get_status_code().equals("304")){
    body_encoding_ = http::NONE;
    ignore_body_encoding_headers_ = true;
  }
}

http::BODY_ENCODING header_handler::get_body_encoding()
{
  return body_encoding_;
}

unsigned int header_handler::get_message_size()
{
  return message_size_;
}

message_buffer& header_handler::get_message_buffer()
{
  return message_buffer_;
}

void header_handler::log_error(char* error)
{
  if(stream_type_ == REQUEST){
    log_info("Request", error);
  }
  else if(stream_type_ == RESPONSE){
    log_info("Response", error);
  }
  else{
    CHECK_CONDITION(false, "invalid stream type; should be REQUEST or RESPONSE");
  }
}

} //namespace http
} //namespace switchflow
