//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_COMBINEDLOGRECORD_HPP
#define SSD_COMBINEDLOGRECORD_HPP

#include <string>

#include <proxylib/i_pipeline_data.hpp>
class combined_log_record: public proxylib::i_pipeline_data
{
public:
  combined_log_record();
  virtual ~combined_log_record();
  void reset();
  void set_dash();
  void set_time();
  
  std::string remote_ip;
  std::string remote_logname;
  std::string user;
  std::string time;
  std::string requestline;
  std::string status;
  std::string bytes_sent;
  std::string referer;
  std::string user_agent;
};



#endif // COMBINEDLOGRECORD_HPP
