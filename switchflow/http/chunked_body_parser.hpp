//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_CHUNKEDBODYPARSER_HPP
#define SF_CHUNKEDBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include "http.hpp"

namespace switchflow{
namespace http{

class i_body_receiver;
  
class chunked_body_parser: private boost::noncopyable
{
public:
  chunked_body_parser(i_body_receiver* p_body_receiver, unsigned int max_chunksize_length);
  virtual ~chunked_body_parser();
  
  void reset();
  STATUS parse_body(read_write_buffer& buffer);
  
private:
  enum STATE
  {
    PARSE_CHUNKSIZE,
    PARSE_CHUNK,
    PARSE_CHUNKSIZE_LF,
    FORWARD_CHUNKSIZE,
    FORWARD_INCOMPLETE_CHUNK,
    FORWARD_COMPLETE_CHUNK,
    PARSE_TRAILER_CR,
    PARSE_TRAILER_LF,
    FORWARD_TRAILER_CRLF
  };

  STATUS parse_chunk_size(read_write_buffer& buffer);
  
  STATE state_;
  
  i_body_receiver* p_body_receiver_;
  
  unsigned int chunksize_;

  unsigned int current_chunksize_length_;

  unsigned int max_chunksize_length_;

  unsigned int current_offset_;

  unsigned int current_chunk_length_;

  unsigned int num_chunk_size_spaces_;
};
} // namespace http
} // namespace switchflow
  
#endif 
