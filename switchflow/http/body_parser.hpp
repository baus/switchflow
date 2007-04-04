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

  class body_parser: private boost::noncopyable
  {
  public:
    body_parser(i_body_receiver* p_body_receiver);

    virtual ~body_parser();

    STATUS parse_body(read_write_buffer& buffer);
    
    void reset(BODY_ENCODING body_encoding, int message_size);
    
    BODY_ENCODING encoding(){return body_encoding_;}
  private:
    BODY_ENCODING body_encoding_;    
    
    chunked_body_parser chunked_body_parser_;
    content_length_body_parser content_length_body_parser_;
    end_connection_body_parser end_connection_body_parser_;
  };
  
} //namespace httplib

#endif // BODYPARSER_H
