#ifndef SSD_I_CLIENT_HPP
#define SSD_I_CLIENT_HPP

#include <util/read_write_buffer.hpp>
#include <socketlib/status.hpp>
#include <socketlib/connection.hpp>

namespace clientlib{

  class client_handler;

}

class i_client
{
public:
  virtual ~i_client(){}
  virtual socketlib::STATUS handle_stream(socketlib::connection& conn) = 0;
  virtual void connect(socketlib::connection& conn) = 0;
  virtual void timeout() = 0;
  virtual void shutdown() = 0;
  virtual void connect_failed() = 0;
  virtual void dns_failed() = 0;
};


#endif // I_CLIENT_HPP
