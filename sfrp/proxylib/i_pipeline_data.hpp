// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_I_PIPELINE_DATA_HPP
#define SSD_I_PIPELINE_DATA_HPP


class i_pipeline_data
{
public:
  virtual ~i_pipeline_data(){}
  virtual void reset() = 0;
};


#endif
