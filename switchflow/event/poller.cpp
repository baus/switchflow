//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <event.h>

#include <util/logger.hpp>

#include "poller.hpp"
#include "event.hpp"
#include "i_event_handler.hpp"

namespace eventlib{

poller::poller()
{
  event_init();
}


poller::~poller()
{
}

void poller::event_callback(int fd, short eventId, void *arg)
{  
  event* pevent = static_cast<event*>(arg);
  pevent->p_event_handler_->handle_event(fd, eventId, *pevent);
}

void poller::add_event(event& r_event, short eventId, long timeoutsecs)
{
  if(pending(r_event)){
    del_event(r_event);
    CHECK_CONDITION(!pending(r_event), "event still pending after deletion");
  }
  int flags = O_RDWR | O_NONBLOCK | O_ASYNC;

  if (fcntl(r_event.fd_, F_SETFL, flags) < 0) {
    log_info("fcntl() returns error");
    return;
  }

  if(timeoutsecs > 0){
    eventId != EV_TIMEOUT;
  }
  event_set(&r_event.event_, r_event.fd_, eventId, event_callback, &r_event);
  if(timeoutsecs > 0){
    r_event.timeout_.tv_sec = timeoutsecs;
    r_event.timeout_.tv_usec = 0;
    event_add(&(r_event.event_), &(r_event.timeout_));
    assert(pending(r_event));
  }
  else{
    event_add(&(r_event.event_), 0);
    assert(pending(r_event));
  }
}

bool poller::pending(event& r_event)
{
  return event_pending(&r_event.event_, EV_TIMEOUT|EV_READ|EV_WRITE|EV_SIGNAL, 0) != 0;
}

void poller::del_event(event& r_event)
{
  if(r_event.fd_ != -1 && pending(r_event)){
    event_del(&r_event.event_);
    assert(!pending(r_event));
  }
}

void poller::dispatch()
{
  int returnVal = event_dispatch();
}
}
