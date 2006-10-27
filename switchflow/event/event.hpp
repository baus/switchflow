//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_EVENT_HPP
#define SSD_EVENT_HPP

#include <sys/time.h>
#include <sys/types.h>
#include <event.h>

typedef struct event event_s;
namespace eventlib
{

class i_event_handler;  

class event
{
public:
  friend class poller;
  event();

  //
  // set function does initialization instead of constructor
  // to allow object recycling.
  int set(int fd, i_event_handler* p_event_handler);

  //
  // Non-virtual.  Do not inherit from event.  Inherit
  // from i_event_handler.
  ~event();
  
private:
  event_s event_;
  int fd_;
  i_event_handler* p_event_handler_;
  timeval timeout_;
};

}

#endif // EVENT_HPP
