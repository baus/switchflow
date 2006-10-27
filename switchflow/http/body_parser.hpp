//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_BODYPARSER_HPP
#define SSD_BODYPARSER_HPP

#include <memory>
#include <boost/noncopyable.hpp>

#include <util/read_write_buffer.hpp>

#include "http.hpp"
#include "chunked_body_parser.hpp"
#include "content_length_body_parser.hpp"
#include "end_connection_body_parser.hpp"


namespace http{

  class BodyParser: private boost::noncopyable
  {
  public:
    BodyParser(i_body_receiver* pBodyReceiver);

    virtual ~BodyParser();

    STATUS parseBody(read_write_buffer& buffer);
    
    void reset(BODY_ENCODING bodyEncoding, int messageSize);
    
    BODY_ENCODING encoding(){return m_bodyEncoding;}
  private:
    BODY_ENCODING m_bodyEncoding;    
    
    ChunkedBodyParser m_chunkedBodyParser;
    ContentLengthBodyParser m_contentLengthBodyParser;
    EndConnectionBodyParser m_endConnectionBodyParser;
  };
  
} //namespace httplib

#endif // BODYPARSER_H
