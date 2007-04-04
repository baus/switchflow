//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.
// 
// Implementation of combined_log_record class

#include "combined_log_record.hpp"
#include <string>


//
// @class combined_log_record combined_log_record.hpp

combined_log_record::combined_log_record()
{
  reset();
}

combined_log_record::~combined_log_record()
{
}

void combined_log_record::set_dash()
{
  if(remote_ip.size() == 0){
    remote_ip = "-";
  }
  if(remote_logname.size() == 0){
    remote_logname = "-";
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
  if(bytes_sent.size() == 0){
    bytes_sent = "-";
  }
  if(referer.size() == 0){
    referer = "-";
  }
  if(user_agent.size() == 0){
    user_agent = "-";
  }
}

void combined_log_record::reset()
{
  remote_ip.clear();
  remote_logname.clear();
  user.clear();
  time.clear();
  requestline.clear();
  status.clear();
  bytes_sent.clear();
  referer.clear();
  user_agent.clear();
}

void combined_log_record::set_time()
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


