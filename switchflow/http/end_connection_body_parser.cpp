//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>

#include "i_body_receiver.hpp"

#include "end_connection_body_parser.hpp"

namespace http{
  
  end_connection_body_parser::end_connection_body_parser(i_body_receiver* p_body_receiver):
  p_body_receiver_(p_body_receiver)
{
}

end_connection_body_parser::~end_connection_body_parser()
{
}

STATUS end_connection_body_parser::parse_end_connection_body(read_write_buffer& buffer)
{
  STATUS return_value = p_body_receiver_->set_body(buffer, false);

  if(return_value == COMPLETE){
    //
    // We might have successfully forwarded the data, but we
    // aren't complete until the connection has been closed.
    //
    return_value = INCOMPLETE;
  }
  else if(return_value == INCOMPLETE){
    return_value = WRITE_INCOMPLETE;
  }
  return return_value; 
}

} // namespace httplib
