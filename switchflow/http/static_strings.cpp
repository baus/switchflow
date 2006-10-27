//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <http/http.hpp>

#include "static_strings.hpp"

namespace http{

static_strings strings_;

  static_strings::static_strings():space_(1),
                                   endline_(2),
                                   fieldsep_(2)
{
  init_raw_buffer(space_, " ");
  init_raw_buffer(fieldsep_, ": ");
  endline_[0] = CR;
  endline_[1] = LF;
}

}
