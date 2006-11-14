//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <boost/config.hpp>
#ifndef BOOST_WINDOWS
#include <syslog.h>
#endif

#include <iostream>
#include "logger.hpp"

//
// Initialize logging functionality.
//
// @param access_log_location the path to the access_log.
// @return 0 on success, -1 on failure.
int logger_init(const char* appname)
{
#ifndef BOOST_WINDOWS
  ::openlog(appname, LOG_NDELAY, LOG_DAEMON);
#endif
  return 1;
}

void logger_shutdown()
{
#ifndef BOOST_WINDOWS
  closelog();
#endif

}

void check_condition(const char * p_file_name, unsigned int src_line, const char* desc)
{
  std::cout<<"Pre-condition failed: "<<p_file_name<<", line "<<src_line<<std::endl;
  std::cout<<desc<<std::endl;
  log_crit(desc);
  abort();
}

void check_condition_val(const char *p_file_name, unsigned int src_line, const char* desc, const int err_val)
{
  std::cout<<"Pre-condition failed: "<<p_file_name<<", line "<<src_line<<std::endl;
  std::cout<<desc<<": "<<err_val<<std::endl;
  log_crit(desc, err_val);
  abort();
}

//
// This should be used if an assertion fails.
void log_crit(const char *message)
{
#ifndef BOOST_WINDOWS

  syslog(LOG_EMERG, message);
#endif

}
void log_crit(const char *message, int error_val)
{
#ifndef BOOST_WINDOWS

  syslog(LOG_EMERG, "%s: %d", message, error_val);  
#endif

}


void log_error(const char *message, int error_val)
{
  std::cerr<<message<<": "<<error_val<<std::endl;
#ifndef BOOST_WINDOWS


  syslog(LOG_ERR, "%s: %d", message, error_val);  
#endif

}

//
// non critical failure: ie log file is bad
void log_error(const char *message)
{
  std::cerr<<message<<std::endl;
#ifndef BOOST_WINDOWS
  syslog(LOG_ERR, message);
#endif

}

//
// Information that might interest user, but isn't a problem
void log_info(const char *message)
{
#ifndef BOOST_WINDOWS
  syslog(LOG_INFO, message);
#endif

}

void log_info(const char *message, int error_val)
{
#ifndef BOOST_WINDOWS

  syslog(LOG_INFO, "%s: %d", message, error_val);  
#endif

}

void log_info(const char *message1, const char *message2)
{
#ifndef BOOST_WINDOWS


  syslog(LOG_INFO, "%s %s", message1, message2);  
#endif

}


//
// Information that really should help us debug internally.  Will not
// display in release mode.
void log_debug(const char *message)
{
#ifndef BOOST_WINDOWS

#ifdef DEBUG
  syslog(LOG_DEBUG, message);
#endif
#endif

}

//
// log request status.
void log_request(const char *message)
{
#ifndef BOOST_WINDOWS


  syslog(LOG_NOTICE, message);
#endif

}

  

void log_debug(const char* message, int val)
{
#ifndef BOOST_WINDOWS


#ifdef DEBUG
  syslog(LOG_INFO, "%s: %d", message, val);
#endif
#endif

}


