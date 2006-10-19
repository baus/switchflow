//
// Copyright (C) Christopher Baus.  All rights reserved.
#include "pipeline_data_factory.hpp"
#include "pipeline_data.hpp"

i_pipeline_data* create_pipeline_data()
{
  return new pipeline_data();
}
