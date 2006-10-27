//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
#include "pipeline_data_factory.hpp"
#include "pipeline_data.hpp"

i_pipeline_data* create_pipeline_data()
{
  return new pipeline_data();
}
