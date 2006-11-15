//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_PIPELINE_DATA_HPP
#define SSD_PIPELINE_DATA_HPP

#include <boost/noncopyable.hpp>

#include <proxylib/i_pipeline_data.hpp>
#include "combined_log_record.hpp"

class pipeline_data: public proxylib::i_pipeline_data
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
  
  combined_log_record logrecord_;
  PROCESS_TYPE process_type_;
  bool request_complete_;
};


#endif // PIPELINE_DATA_HPP
