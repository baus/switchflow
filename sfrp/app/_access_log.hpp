//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.
// 
// Definition of access_log class

#ifndef SSD_ACCESSLOG_HPP
#define SSD_ACCESSLOG_HPP

#include "combined_log_record.hpp"

class access_log
{
public:
  access_log();
  virtual ~access_log();

  bool open(const char* filename);

  bool is_open();
  
  //
  // log to access log.
  void log_access(combined_log_record& log_record);

private:
  int logfd_;

};


#endif // ACCESSLOG_HPP
