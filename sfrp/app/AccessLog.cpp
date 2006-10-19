// 
// Implementation of AccessLog class
//
// root <christopher@baus.net>
//
// Copyright (C) Summit Sage, LLC.  All rights reserved.
//
#include "AccessLog.hpp"

#include <syslog.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

//
// @class AccessLog accesslog.hpp
//
// @author <a href="mailto:christopher@baus.net">root</a>
//

AccessLog::AccessLog():m_logfd(-1)
{
}

void AccessLog::logAccess(CombinedLogRecord& logRecord)
{
  static char *space = " ";
  static char *quote = "\"";
  static char *newline = "\n";
  if(m_logfd == -1){
    return;
  }
  logRecord.setTime();
  logRecord.setDash();
  
  unsigned int currentVector = 0;
    
  iovec vector[24];
    
  vector[currentVector].iov_base = const_cast<char*>(logRecord.remoteIP.c_str());
  vector[currentVector].iov_len = logRecord.remoteIP.length();
  ++currentVector;
    
  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  vector[currentVector].iov_base = const_cast<char*>(logRecord.remoteLogname.c_str());
  vector[currentVector].iov_len = logRecord.remoteLogname.length();
  ++currentVector;

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  vector[currentVector].iov_base = const_cast<char*>(logRecord.user.c_str());
  vector[currentVector].iov_len = logRecord.user.length();
  ++currentVector;

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  vector[currentVector].iov_base = const_cast<char*>(logRecord.time.c_str());
  vector[currentVector].iov_len = logRecord.time.length();
  ++currentVector;

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  if(logRecord.requestline != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }

  vector[currentVector].iov_base = const_cast<char*>(logRecord.requestline.c_str());
  vector[currentVector].iov_len = logRecord.requestline.length();
  ++currentVector;

  if(logRecord.requestline != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }
  
  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  vector[currentVector].iov_base = const_cast<char*>(logRecord.status.c_str());
  vector[currentVector].iov_len = logRecord.status.length();
  ++currentVector;

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  vector[currentVector].iov_base = const_cast<char*>(logRecord.bytesSent.c_str());
  vector[currentVector].iov_len = logRecord.bytesSent.length();
  ++currentVector;

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  if(logRecord.referer != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }
  vector[currentVector].iov_base = const_cast<char*>(logRecord.referer.c_str());
  vector[currentVector].iov_len = logRecord.referer.length();
  ++currentVector;

  if(logRecord.referer != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }

  vector[currentVector].iov_base = space;
  vector[currentVector].iov_len = strlen(space);
  ++currentVector;

  if(logRecord.userAgent != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }

  vector[currentVector].iov_base = const_cast<char*>(logRecord.userAgent.c_str());
  vector[currentVector].iov_len = logRecord.userAgent.length();
  ++currentVector;

  if(logRecord.userAgent != "-"){
    vector[currentVector].iov_base = quote; 
    vector[currentVector].iov_len = strlen(quote);
    ++currentVector;
  }

  vector[currentVector].iov_base = newline; 
  vector[currentVector].iov_len = strlen(quote);
  ++currentVector;

  //
  // Feel the performance.  Do it all in one big write.
  //
  ::writev(m_logfd, vector, currentVector);

}


bool AccessLog::open(const char* filename)
{
  m_logfd = ::open(filename, O_WRONLY|O_APPEND|O_CREAT, 00755);
  int error = errno;
  return m_logfd != -1;
}

bool AccessLog::isOpen()
{
  return m_logfd != -1;
}

AccessLog::~AccessLog()
{
  if(m_logfd != -1){
    close(m_logfd);
  }
}
