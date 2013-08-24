//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef INET_ADDR_H
#define INET_ADDR_H


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <boost/asio.hpp>


namespace socketlib{

  class inet_addr
  {
  public:
    inet_addr();
    inet_addr(const inet_addr& rhs);
    inet_addr& operator=(const inet_addr& rhs);
    sockaddr_in& get_addr();
    
    bool resolve( const char* address, int port );
    bool resolve( const char* address, const char* port );

  private:
    sockaddr_in addr_;
    static void copy_address( sockaddr_in& dest, const sockaddr_in& src);

  };

  bool resolve_addr(const char* address, boost::asio::ip::address& addr );

} // namespace socketlib


#endif // SOCKETDATA_H
