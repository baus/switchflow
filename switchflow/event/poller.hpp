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
  
  void add_event(event& r_event, short eventId, long timeoutsecs);
  bool pending(event& r_event);
  void del_event(event& r_event);
  
  void dispatch();
  
private:
  //
  // Static callback function that is passed to libevent. 
  static void event_callback(int fd, short eventId, void *arg);

};

}
#endif // POLLER_HPP
