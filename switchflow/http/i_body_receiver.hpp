//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef HTTP_BODY_RECEIVER_H
#define HTTP_BODY_RECEIVER_H

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace switchflow{
namespace http{
 
class i_body_receiver
{
 public:
  //
  // The implementor shouldn't do anything here that may fail (ie I/O or memory allocation). 
  virtual void set_body_encoding(BODY_ENCODING body_encoding) = 0;

  virtual STATUS set_body(read_write_buffer& buffer, bool b_complete) = 0;

  virtual void set_chunk_size(unsigned int chunk_size) = 0;

  virtual STATUS forward_chunk_size() = 0;

  virtual STATUS forward_chunk_trailer() = 0;
};

} // namespace httplib
} // namespace switchflow

#endif
