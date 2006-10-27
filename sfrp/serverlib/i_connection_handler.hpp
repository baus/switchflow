//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.
//
#ifndef SSD_CONNECTION_HANDLER_HPP
#define SSD_CONNECTION_HANDLER_HPP

#include <event/i_event_handler.hpp>
#include <event/event.hpp>

#include <socketlib/connection.hpp>

namespace serverlib{
  
class i_connection_handler:public eventlib::i_event_handler
{
public:
  i_connection_handler():socket_data_(5000){}
  virtual ~i_connection_handler(){}
  virtual i_connection_handler* clone() = 0;
  void reset(int clientfd);
  eventlib::event client_event_;
  socketlib::connection socket_data_;
};

}

#endif // CONNECT_HANDLER_HPP
