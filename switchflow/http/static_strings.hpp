//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_STATIC_STRINGS_HPP
#define SSD_STATIC_STRINGS_HPP

#include <util/read_write_buffer.hpp>


namespace http{
  
class static_strings
{
public:
  static_strings();

  raw_buffer space_;
  raw_buffer endline_;
  raw_buffer fieldsep_;
};

extern static_strings strings_;

}

#endif // STATIC_STRINGS_HPP
