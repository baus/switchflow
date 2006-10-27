//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Definition of AccessLog class
//
// christopher <christopher@baus.net>
//
// Copyright (C) Summit Sage Designs, LLC.  All rights reserved.
//
#ifndef SSD_ACCESSLOG_HPP
#define SSD_ACCESSLOG_HPP

#include "CombinedLogRecord.hpp"

class AccessLog
{
public:
  AccessLog();
  virtual ~AccessLog();

  bool open(const char* filename);

  bool isOpen();
  
  //
  // log to access log.
  void logAccess(CombinedLogRecord& logRecord);

private:
  int m_logfd;

};


#endif // ACCESSLOG_HPP
