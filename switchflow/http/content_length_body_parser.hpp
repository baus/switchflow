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
    unsigned int contentLength_;
    
    /// The offset into the buffer being processed.
    ///
    unsigned int currentOffset_;
    
    /// The current length of the element being parsed.
    ///
    unsigned int currentLength_;

    /// 
    ///
    PARSE_STATE parseState_;

    i_body_receiver* pBodyReceiver_;
  };
}
#endif // CONTENTLENGTHBODYPARSER_HPP
