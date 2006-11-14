//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_POLLER_HPP
#define SSD_POLLER_HPP

namespace eventlib{
class event;

class poller
{
public:
  poller();
  virtual ~poller();
  
  void add_event(event& r_event, short event_id, long timeoutsecs);
  bool pending(event& r_event);
  void del_event(event& r_event);
  
  void dispatch();
  
private:
  //
  // Static callback function that is passed to libevent. 
  static void event_callback(int fd, short event_id, void *arg);

};

}
#endif // POLLER_HPP
