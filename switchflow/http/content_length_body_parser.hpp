//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_CONTENTLENGTHBODYPARSER_HPP
#define SSD_CONTENTLENGTHBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include <util/logger.hpp>

#include "i_body_receiver.hpp"


namespace http{

  class ContentLengthBodyParser: private boost::noncopyable
  {
  public:
    ContentLengthBodyParser(i_body_receiver* pBodyReceiver);
    virtual ~ContentLengthBodyParser();

    STATUS parseContentLengthBody(read_write_buffer& buffer);

    void reset(unsigned int messageSize);
    
  private:
    enum PARSE_STATE
    {
      MESSAGE_BODY_PARSE,                    
      INCOMPLETE_MESSAGE_BODY_FORWARD,       
      COMPLETE_MESSAGE_BODY_FORWARD          
    };

    void transitionToState(PARSE_STATE newState);
    
    /// The content length passed from the headers.
    ///
    unsigned int m_contentLength;
    
    /// The offset into the buffer being processed.
    ///
    unsigned int m_currentOffset;
    
    /// The current length of the element being parsed.
    ///
    unsigned int m_currentLength;

    /// 
    ///
    PARSE_STATE m_parseState;

    i_body_receiver* m_pBodyReceiver;
  };
}
#endif // CONTENTLENGTHBODYPARSER_HPP
