//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_I_REQUEST_POSTPROCESSOR_HPP
#define SSD_I_REQUEST_POSTPROCESSOR_HPP

#include <http/message_buffer.hpp>

class i_request_postprocessor
{
public:
  virtual ~i_request_postprocessor(){};
  virtual bool process_request(switchflow::http::message_buffer& buffer) = 0;
  
private:
};


#endif // I_REQUEST_POSTPROCESSOR_HPP
