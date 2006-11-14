//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_I_PIPELINE_DATA_HPP
#define SSD_I_PIPELINE_DATA_HPP


class i_pipeline_data
{
public:
  virtual ~i_pipeline_data(){}
  virtual void reset() = 0;
};


#endif
