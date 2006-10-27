//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <assert.h>

#include "i_body_receiver.hpp"

#include "end_connection_body_parser.hpp"

namespace http{
  
  EndConnectionBodyParser::EndConnectionBodyParser(i_body_receiver* pBodyReceiver):
  m_pBodyReceiver(pBodyReceiver)
{
}

EndConnectionBodyParser::~EndConnectionBodyParser()
{
}

STATUS EndConnectionBodyParser::parseEndConnectionBody(read_write_buffer& buffer)
{
  STATUS returnValue = m_pBodyReceiver->set_body(buffer, false);

  if(returnValue == COMPLETE){
    //
    // We might have successfully forwarded the data, but we
    // aren't complete until the connection has been closed.
    //
    returnValue = INCOMPLETE;
  }
  else if(returnValue == INCOMPLETE){
    returnValue = WRITE_INCOMPLETE;
  }
  return returnValue; 
}

} // namespace httplib
