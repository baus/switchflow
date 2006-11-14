//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>
#include <memory>

#include <util/logger.hpp>
#include <util/conversions.hpp>

#include "body_parser.hpp"
#include "http.hpp"

namespace http{
  body_parser::body_parser(i_body_receiver* p_body_receiver):
  chunked_body_parser_(p_body_receiver, CHUNK_SIZE_LENGTH),
  content_length_body_parser_(p_body_receiver),
  end_connection_body_parser_(p_body_receiver)
{
}


body_parser::~body_parser()
{
}

void body_parser::reset(BODY_ENCODING body_encoding,
                       int message_size)
{
  body_encoding_ = body_encoding;
  content_length_body_parser_.reset(message_size);
  chunked_body_parser_.reset();
}


STATUS body_parser::parse_body(read_write_buffer& buffer)
{
  if(body_encoding_ == NONE){
    return COMPLETE;
  }
  if(body_encoding_ == CHUNKED){
    return chunked_body_parser_.parse_body(buffer);
  }
  if(body_encoding_ == END_CONNECTION){
    return end_connection_body_parser_.parse_end_connection_body(buffer);
  }

  assert(body_encoding_ == CONTENT_LENGTH);

  return content_length_body_parser_.parse_content_length_body(buffer);
  
}

} // namespace httplib


