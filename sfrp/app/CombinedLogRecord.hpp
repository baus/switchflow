//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_COMBINEDLOGRECORD_HPP
#define SSD_COMBINEDLOGRECORD_HPP

#include <string>

#include <proxylib/IPipelineData.hpp>
class CombinedLogRecord: public proxylib::IPipelineData
{
public:
  CombinedLogRecord();
  virtual ~CombinedLogRecord();
  void reset();
  void setDash();
  void setTime();
  
  std::string remoteIP;
  std::string remoteLogname;
  std::string user;
  std::string time;
  std::string requestline;
  std::string status;
  std::string bytesSent;
  std::string referer;
  std::string userAgent;
};



#endif // COMBINEDLOGRECORD_HPP
