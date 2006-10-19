#ifndef HTTPBodyReceiver_H
#define HTTPBodyReceiver_H

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace http{
  
class i_body_receiver
{
 public:
  //
  // The implementor shouldn't do anything here that may fail (ie I/O or, forbid, memory allocation).    
  //
  virtual void set_body_encoding(BODY_ENCODING bodyEncoding) = 0;

  virtual STATUS set_body(read_write_buffer& buffer, bool bComplete) = 0;

  virtual void set_chunk_size(unsigned int chunkSize) = 0;

  virtual STATUS forward_chunk_size() = 0;

  virtual STATUS forward_chunk_trailer() = 0;
};

} // namespace httplib

#endif
