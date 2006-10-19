#include "url.hpp"

#include <socketlib/inet_addr.hpp>
#include <socketlib/connection.hpp>
#include <stdio.h>
#include <string>

namespace httplib{

URL::PARSE_STATUS URL::parse_url(const char* URL)
{
  raw_url = URL;
  port = 80;
  const char *cur = URL;
  char buf[4096];
  int indx = 0;
  const int indxMax = 4096 - 1;
  
  
  if (URL == NULL) return INVALID;
  buf[indx] = 0;
  while ((*cur != 0) && (indx < indxMax)) {
    if ((cur[0] == ':') && (cur[1] == '/') && (cur[2] == '/')) {
	    buf[indx] = 0;
	    protocol = buf;
	    indx = 0;
      cur += 3;
	    break;
    }
    buf[indx++] = *cur++;
  }
  if (*cur == 0){
    return NO_PROTOCOL;
  }
  
  buf[indx] = 0;
  while (indx < indxMax) {
    if ((strchr (cur, '[') && !strchr (cur, ']')) ||
		(!strchr (cur, '[') && strchr (cur, ']'))) {
	    return INVALID;
    }
    
    if (cur[0] == '[') {
      //
      // I don't really understand this block what do []'s mean in the URL?
	    cur++;
	    while ((cur[0] != ']') && (indx < indxMax))
        buf[indx++] = *cur++;
      
	    if (!strchr (buf, ':')) {
        return INVALID;
	    }

	    buf[indx] = 0;
	    hostname = buf;
	    indx = 0;
	    cur += 1;
	    if (cur[0] == ':') {
        port = 0;
        cur++;
        while (*cur >= '0' && *cur <= '9') {
          port *= 10;
          port += *cur - '0';
          cur++;
        }
        
        if (port == 0) port = 80;
        while ((cur[0] != '/') && (*cur != 0))
          cur++;
	    }
	    break;
    }
    else {
	    if (cur[0] == ':') {
        buf[indx] = 0;
        hostname = buf;
        indx = 0;
        cur += 1;
        port = 0;
        while ((*cur >= '0') && (*cur <= '9')) {
          port *= 10;
          port += *cur - '0';
          cur++;
        }
        if (port == 0) port = 80;
        while ((cur[0] != '/') && (*cur != 0)) 
          cur++;
        break;
	    }
	    if ((*cur == '/') || (*cur == 0)) {
        buf[indx] = 0;
        hostname = buf;
        indx = 0;
        break;
	    }
    }
    buf[indx++] = *cur++;
  }
  if (*cur == 0) {
    path = "/";
  }
  else {
    indx = 0;
    buf[indx] = 0;
    while ((*cur != 0) && (indx < indxMax))
	    buf[indx++] = *cur++;
    buf[indx] = 0;
    path = buf;
  }
  return VALID;
}

URL::URL()
{
}

URL::URL(const char* URL, bool resolve):found_host(false)
{
  parse_url(URL);
  if(resolve){
    resolve_addr();
  }
}

URL::URL(const URL& rhs):
  endpoint(rhs.endpoint),
  protocol(rhs.protocol),
  hostname(rhs.hostname),
  port(rhs.port),
  path(rhs.path),
  found_host(rhs.found_host),
  raw_url(rhs.raw_url)
{
}

bool URL::resolve_addr()
{
  asio::ip::address addr;
  socketlib::resolve_addr(hostname.c_str(), addr);
  endpoint = asio::ip::tcp::endpoint(addr, port);
}

std::string URL::host_with_port()
{
  std::string host = hostname;
  if(port != 80){
    char buf[50];
    host += ":";
    snprintf(buf, 50, "%i", port); 
    host += buf;
  }
  return host;
}

URL::~URL()
{
}

}
