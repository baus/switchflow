// 
// Implementation of CombinedLogRecord class
//
// christopher <christopher@baus.net>
//
// Copyright (C) Summit Sage Designs, LLC.  All rights reserved.
//
#include "CombinedLogRecord.hpp"
#include <string>


//
// @class CombinedLogRecord combinedlogrecord.hpp
//
// @author <a href="mailto:christopher@baus.net">christopher</a>
//

CombinedLogRecord::CombinedLogRecord()
{
  reset();
}

CombinedLogRecord::~CombinedLogRecord()
{
}

void CombinedLogRecord::setDash()
{
  if(remoteIP.size() == 0){
    remoteIP = "-";
  }
  if(remoteLogname.size() == 0){
    remoteLogname = "-";
  }
  if(user.size() == 0){
    user = "-";
  }
  if(time.size() == 0){
    time = "-";
  }
  if(requestline.size() == 0){
    requestline = "-";
  }
  if(status.size() == 0){
    status = "-";
  }
  if(bytesSent.size() == 0){
    bytesSent = "-";
  }
  if(referer.size() == 0){
    referer = "-";
  }
  if(userAgent.size() == 0){
    userAgent = "-";
  }
}

void CombinedLogRecord::reset()
{
  remoteIP.clear();
  remoteLogname.clear();
  user.clear();
  time.clear();
  requestline.clear();
  status.clear();
  bytesSent.clear();
  referer.clear();
  userAgent.clear();
}

void CombinedLogRecord::setTime()
{
  tm *date;   // Date/time value
  time_t t;
  char s[1024]; // Date/time string

  static const char * const months[12] =
    {   /* Months */
      "Jan",
      "Feb",
      "Mar",
      "Apr",
      "May",
      "Jun",
      "Jul",
      "Aug",
      "Sep",
      "Oct",
      "Nov",
      "Dec"
    };


  ::time(&t);

  /*
   * Get the date and time from the UNIX time value, and then format it
   * into a string.  Note that we *can't* use the strftime() function since
   * it is localized and will seriously confuse automatic programs if the
   * month names are in the wrong language!
   *
   * Also, we use the "timezone" variable that contains the current timezone
   * offset from GMT in seconds so that we are reporting local time in the
   * log files.  If you want GMT, set the TZ environment variable accordingly
   * before starting the scheduler.
   *
   * (*BSD and Darwin store the timezone offset in the tm structure)
   */

  date = localtime(&t);
  
  snprintf(s, sizeof(s), "[%02d/%s/%04d:%02d:%02d:%02d %+03ld%02ld]",
           date->tm_mday, months[date->tm_mon], 1900 + date->tm_year,
           date->tm_hour, date->tm_min, date->tm_sec,
#define HAVE_TM_GMTOFF
#ifdef HAVE_TM_GMTOFF
           date->tm_gmtoff / 3600, (date->tm_gmtoff / 60) % 60);
#else
  timezone / 3600, (timezone / 60) % 60);
#endif /* HAVE_TM_GMTOFF */

  time = s;
}


