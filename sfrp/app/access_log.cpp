//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.
// 
// Implementation of access_log class

#include "access_log.hpp"

#include <syslog.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

//
// @class access_log access_log.hpp
//
// @author <a href="mailto:christopher@baus.net">root</a>
//

access_log::access_log():logfd_(-1)
{
}

void access_log::log_access(combined_log_record& log_record)
{
  static char *space = " ";
  static char *quote = "\"";
  static char *newline = "\n";
  if(logfd_ == -1){
    return;
  }
  log_record.set_time();
  log_record.set_dash();
  
  unsigned int current_vector = 0;
    
  iovec vector[24];
    
  vector[current_vector].iov_base = const_cast<char*>(log_record.remote_ip.c_str());
  vector[current_vector].iov_len = log_record.remote_ip.length();
  ++current_vector;
    
  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  vector[current_vector].iov_base = const_cast<char*>(log_record.remote_logname.c_str());
  vector[current_vector].iov_len = log_record.remote_logname.length();
  ++current_vector;

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  vector[current_vector].iov_base = const_cast<char*>(log_record.user.c_str());
  vector[current_vector].iov_len = log_record.user.length();
  ++current_vector;

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  vector[current_vector].iov_base = const_cast<char*>(log_record.time.c_str());
  vector[current_vector].iov_len = log_record.time.length();
  ++current_vector;

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  if(log_record.requestline != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }

  vector[current_vector].iov_base = const_cast<char*>(log_record.requestline.c_str());
  vector[current_vector].iov_len = log_record.requestline.length();
  ++current_vector;

  if(log_record.requestline != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }
  
  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  vector[current_vector].iov_base = const_cast<char*>(log_record.status.c_str());
  vector[current_vector].iov_len = log_record.status.length();
  ++current_vector;

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  vector[current_vector].iov_base = const_cast<char*>(log_record.bytes_sent.c_str());
  vector[current_vector].iov_len = log_record.bytes_sent.length();
  ++current_vector;

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  if(log_record.referer != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }
  vector[current_vector].iov_base = const_cast<char*>(log_record.referer.c_str());
  vector[current_vector].iov_len = log_record.referer.length();
  ++current_vector;

  if(log_record.referer != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }

  vector[current_vector].iov_base = space;
  vector[current_vector].iov_len = strlen(space);
  ++current_vector;

  if(log_record.user_agent != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }

  vector[current_vector].iov_base = const_cast<char*>(log_record.user_agent.c_str());
  vector[current_vector].iov_len = log_record.user_agent.length();
  ++current_vector;

  if(log_record.user_agent != "-"){
    vector[current_vector].iov_base = quote; 
    vector[current_vector].iov_len = strlen(quote);
    ++current_vector;
  }

  vector[current_vector].iov_base = newline; 
  vector[current_vector].iov_len = strlen(quote);
  ++current_vector;

  //
  // Feel the performance.  Do it all in one big write.
  //
  ::writev(logfd_, vector, current_vector);

}


bool access_log::open(const char* filename)
{
  logfd_ = ::open(filename, O_WRONLY|O_APPEND|O_CREAT, 00755);
  int error = errno;
  return logfd_ != -1;
}

bool access_log::is_open()
{
  return logfd_ != -1;
}

access_log::~access_log()
{
  if(logfd_ != -1){
    close(logfd_);
  }
}
