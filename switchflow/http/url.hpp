//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSVS_URL
#define SSVS_URL

#include <string>

#include <asio.hpp>

namespace httplib{

class url
{
 public:
  enum PARSE_STATUS{
    VALID,
    NO_PROTOCOL,
    INVALID
  };
  
  url();
  url(const char* URL, bool resolve);
  url(const url& rhs);
  std::string host_with_port();
  ~url();
  bool resolve_addr();
  
  asio::ip::tcp::endpoint endpoint;
  std::string raw_url;
  std::string protocol; // the protocol name
  std::string hostname; // the host name
  unsigned short port;              // the port
  std::string path;     // the path within the URL
  bool found_host;

  PARSE_STATUS parse_url(const char* URL);

private:
};  

}
#endif
