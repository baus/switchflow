//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_ENDCONNECTIONBODYPARSER_HPP
#define SF_ENDCONNECTIONBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include <util/logger.hpp>

#include "http.hpp"


namespace switchflow{
namespace http{
  class i_body_receiver;
  
  class end_connection_body_parser: private boost::noncopyable
  {
  public:
    end_connection_body_parser(i_body_receiver* p_body_receiver);
    virtual ~end_connection_body_parser();

    STATUS parse_end_connection_body(read_write_buffer& buffer);
    
  private:
    i_body_receiver* p_body_receiver_;
  };

} // namespace http
} // namespace switchflow

#endif 
