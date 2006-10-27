//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
#ifndef SSD_PIPELINE_DATA_HPP
#define SSD_PIPELINE_DATA_HPP

#include <boost/noncopyable.hpp>

#include <proxylib/i_pipeline_data.hpp>
#include "CombinedLogRecord.hpp"

class pipeline_data: public i_pipeline_data
{
public:
  enum PROCESS_TYPE{
    NORMAL,
    HEAD,
    DENY
  };
    
  pipeline_data();
  virtual ~pipeline_data();
  void reset();
  
  CombinedLogRecord logrecord_;
  PROCESS_TYPE process_type_;
  bool request_complete_;
};


#endif // PIPELINE_DATA_HPP
