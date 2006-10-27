//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <netdb.h>

#include <stdlib.h>
#include <strings.h>

#include "inet_addr.hpp"

namespace socketlib{

  inet_addr::inet_addr()
  {
    bzero(&addr_, sizeof(struct sockaddr_in));
  }

  inet_addr::inet_addr(const inet_addr& rhs)
  {
    copy_address(addr_, rhs.addr_);
  }
  
  inet_addr& inet_addr::operator=(const inet_addr& rhs)
  {
    copy_address(addr_, rhs.addr_);
    return *this;
  }

  sockaddr_in& inet_addr::get_addr()
  {
    return addr_;
  }

  bool inet_addr::resolve( const char* address, int port ) 
  {
    struct hostent* hp;
    bzero( &addr_, sizeof(struct sockaddr_in) );
    hp = gethostbyname( address );
    if(NULL == hp){
      return false;
    }
    addr_.sin_addr = *((struct in_addr *)hp->h_addr);
    addr_.sin_family = hp->h_addrtype;
    addr_.sin_port = htons( port );
    return true;
  }

  //
  bool inet_addr::resolve( const char* address, const char* port ) 
  {
#warning should check for valid port with something safer than atoi  
    return resolve(address, atoi(port));
  }

  
  void inet_addr::copy_address( sockaddr_in& dest, const sockaddr_in& src)
  {
    bzero(&dest, sizeof(struct sockaddr_in));
    dest.sin_family = src.sin_family; 
    dest.sin_port   = src.sin_port;
    dest.sin_addr   = src.sin_addr;   
  }

  bool resolve_addr(const char* address, asio::ip::address& addr )
  {
    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    asio::ip::tcp::resolver::query query(address, "0");
    asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    asio::ip::tcp::resolver::iterator end;
    if(iter != end){
      asio::ip::tcp::endpoint ep = *iter;
      addr = ep.address();
      return true;
    }
    return false;
  }


  
}
