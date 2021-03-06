//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_I_EVENT_HANDLER_HPP
#define SSD_I_EVENT_HANDLER_HPP

namespace eventlib{
class poller;
class event;

class i_event_handler
{
public:
  virtual int handle_event(int fd, short revent, event& event) = 0;
};

}

#endif // I_EVENT_HANDLER_HPP
