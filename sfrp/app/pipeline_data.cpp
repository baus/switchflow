// 
// Copyright (C) Christopher Baus.  All rights reserved.
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
