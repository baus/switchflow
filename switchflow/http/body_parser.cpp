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
  m_chunkedBodyParser(pBodyReceiver, CHUNK_SIZE_LENGTH),
  m_contentLengthBodyParser(pBodyReceiver),
  m_endConnectionBodyParser(pBodyReceiver)
{
}


BodyParser::~BodyParser()
{
}

void BodyParser::reset(BODY_ENCODING bodyEncoding,
                       int messageSize)
{
  m_bodyEncoding = bodyEncoding;
  m_contentLengthBodyParser.reset(messageSize);
  m_chunkedBodyParser.reset();
}


STATUS BodyParser::parseBody(read_write_buffer& buffer)
{
  if(m_bodyEncoding == NONE){
    return COMPLETE;
  }
  if(m_bodyEncoding == CHUNKED){
    return m_chunkedBodyParser.parseBody(buffer);
  }
  if(m_bodyEncoding == END_CONNECTION){
    return m_endConnectionBodyParser.parseEndConnectionBody(buffer);
  }

  assert(m_bodyEncoding == CONTENT_LENGTH);

  return m_contentLengthBodyParser.parseContentLengthBody(buffer);
  
}

} // namespace httplib


