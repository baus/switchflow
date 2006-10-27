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
// @param accessLogLocation the path to the accessLog.
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

void check_condition(const char * pFileName, unsigned int SrcLine, const char* Desc)
{
  std::cout<<"Pre-condition failed: "<<pFileName<<", line "<<SrcLine<<std::endl;
  std::cout<<Desc<<std::endl;
  log_crit(Desc);
  abort();
}

void check_condition_val(const char *pFileName, unsigned int SrcLine, const char* Desc, const int ErrVal)
{
  std::cout<<"Pre-condition failed: "<<pFileName<<", line "<<SrcLine<<std::endl;
  std::cout<<Desc<<": "<<ErrVal<<std::endl;
  log_crit(Desc, ErrVal);
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
void log_crit(const char *message, int errorVal)
{
#ifndef BOOST_WINDOWS

  syslog(LOG_EMERG, "%s: %d", message, errorVal);  
#endif

}


void log_error(const char *message, int errorVal)
{
  std::cerr<<message<<": "<<errorVal<<std::endl;
#ifndef BOOST_WINDOWS


  syslog(LOG_ERR, "%s: %d", message, errorVal);  
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

void log_info(const char *message, int errorVal)
{
#ifndef BOOST_WINDOWS

  syslog(LOG_INFO, "%s: %d", message, errorVal);  
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

  

void logDebug(const char* message, int val)
{
#ifndef BOOST_WINDOWS


#ifdef DEBUG
  syslog(LOG_INFO, "%s: %d", message, val);
#endif
#endif

}


