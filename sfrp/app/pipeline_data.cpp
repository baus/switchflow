//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "pipeline_data.hpp"


pipeline_data::pipeline_data()
{
  reset();
}

void pipeline_data::reset()
{
  logrecord_.reset();
  process_type_ = NORMAL;
  request_complete_ = false;
}

pipeline_data::~pipeline_data()
{
}
