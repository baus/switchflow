#ifndef I_CONNECT_HANDLER_H
#define I_CONNECT_HANDLER_H

#include <memory>

class i_connect_error_handler
{
public:
  virtual void timeout() = 0;
  virtual void connect_failed() = 0;
  virtual void dns_failed() = 0;
};

class i_connect_handler
{
public:
  
  virtual void connect(std::auto_ptr<asio::ip::tcp::socket> p_stream_socket_) = 0;
};


#endif

