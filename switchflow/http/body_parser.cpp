//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>
#include <memory>

#include <util/logger.hpp>
#include <util/conversions.h>

#include "body_parser.hpp"
#include "http.hpp"

namespace http{
  BodyParser::BodyParser(i_body_receiver* pBodyReceiver):
  chunkedBodyParser_(pBodyReceiver, CHUNK_SIZE_LENGTH),
  contentLengthBodyParser_(pBodyReceiver),
  endConnectionBodyParser_(pBodyReceiver)
{
}


BodyParser::~BodyParser()
{
}

void BodyParser::reset(BODY_ENCODING bodyEncoding,
                       int messageSize)
{
  bodyEncoding_ = bodyEncoding;
  contentLengthBodyParser_.reset(messageSize);
  chunkedBodyParser_.reset();
}


STATUS BodyParser::parseBody(read_write_buffer& buffer)
{
  if(bodyEncoding_ == NONE){
    return COMPLETE;
  }
  if(bodyEncoding_ == CHUNKED){
    return chunkedBodyParser_.parseBody(buffer);
  }
  if(bodyEncoding_ == END_CONNECTION){
    return endConnectionBodyParser_.parseEndConnectionBody(buffer);
  }

  assert(bodyEncoding_ == CONTENT_LENGTH);

  return contentLengthBodyParser_.parseContentLengthBody(buffer);
  
}

} // namespace httplib


