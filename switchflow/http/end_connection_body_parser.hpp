//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_ENDCONNECTIONBODYPARSER_HPP
#define SSD_ENDCONNECTIONBODYPARSER_HPP

#include <boost/noncopyable.hpp>

#include <util/logger.hpp>

#include "http.hpp"


namespace http{
  class i_body_receiver;
  
  class EndConnectionBodyParser: private boost::noncopyable
  {
  public:
    EndConnectionBodyParser(i_body_receiver* pBodyReceiver);
    virtual ~EndConnectionBodyParser();

    STATUS parseEndConnectionBody(read_write_buffer& buffer);
    
  private:
    i_body_receiver* pBodyReceiver_;
  };

} // namespace httplib

#endif // ENDCONNECTIONBODYPARSER_HPP
