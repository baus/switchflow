#ifndef SSD_CHUNKEDBODYPARSER_HPP
#define SSD_CHUNKEDBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include "http.hpp"


namespace http{

class i_body_receiver;
  
class ChunkedBodyParser: private boost::noncopyable
{
public:
  ChunkedBodyParser(i_body_receiver* pBodyReceiver, unsigned int maxChunksizeLength);
  virtual ~ChunkedBodyParser();
  
  void reset();
  STATUS parseBody(read_write_buffer& buffer);
  
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

  STATUS parseChunkSize(read_write_buffer& buffer);
  
  STATE m_state;
  
  i_body_receiver* m_pBodyReceiver;
  
  unsigned int m_chunksize;

  unsigned int m_currentChunksizeLength;

  unsigned int m_maxChunksizeLength;

  unsigned int m_currentOffset;

  unsigned int m_currentChunkLength;

  unsigned int m_numChunkSizeSpaces;
};

}// namespace httplib
  
#endif // CHUNKEDBODYPARSER_HPP
