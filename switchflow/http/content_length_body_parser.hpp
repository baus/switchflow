//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_CONTENTLENGTHBODYPARSER_HPP
#define SF_CONTENTLENGTHBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include <util/logger.hpp>

#include "i_body_receiver.hpp"

namespace switchflow{
namespace http{

  class content_length_body_parser: private boost::noncopyable
  {
  public:
    content_length_body_parser(i_body_receiver* p_body_receiver);
    virtual ~content_length_body_parser();

    STATUS parse_content_length_body(read_write_buffer& buffer);

    void reset(unsigned int message_size);
    
  private:
    enum PARSE_STATE
    {
      MESSAGE_BODY_PARSE,                    
      INCOMPLETE_MESSAGE_BODY_FORWARD,       
      COMPLETE_MESSAGE_BODY_FORWARD          
    };

    void transition_to_state(PARSE_STATE new_state);
    
    /// The content length passed from the headers.
    ///
    unsigned int content_length_;
    
    /// The offset into the buffer being processed.
    ///
    unsigned int current_offset_;
    
    /// The current length of the element being parsed.
    ///
    unsigned int current_length_;

    /// 
    ///
    PARSE_STATE parse_state_;

    i_body_receiver* p_body_receiver_;
  };
  
} //namespace http
} //namespace switchflow

#endif 
