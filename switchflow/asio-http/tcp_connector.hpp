#ifndef TCP_CONNECTOR_H
#define TCP_CONNECTOR_H

#include <boost/noncopyable.hpp>
#include <asio.hpp>
#include <boost/function.hpp>

#include <memory>
#include "i_connect_handler.hpp"

  
  
// performs a non-blocking connect, and calls
// an event handler when the socket is connected.  
//
class tcp_connector: private boost::noncopyable
{
 public:
  tcp_connector(asio::io_service& d,
              asio::ip::tcp::endpoint endpoint,
        boost::function< void (std::auto_ptr<asio::ip::tcp::socket>) > connect_handler,
                i_connect_error_handler& connect_error_handler);

  tcp_connector(asio::io_service& d,
                const char* address,
                int port,
        boost::function<void (std::auto_ptr<asio::ip::tcp::socket>) > connect_handler,
                i_connect_error_handler& connect_error_handler);
  
  //
  // This class shouldn't be subclassed.
  ~tcp_connector(){}
  
  void resolve_handler(const asio::error& error, asio::ip::tcp::resolver::iterator iter);

  void connect_handler(const asio::error& error);

private:

  void connect();
  
  asio::io_service& d_;
  
  tcp_connector();

  asio::ip::tcp::endpoint endpoint_;

  i_connect_error_handler& connect_error_handler_;

  asio::ip::tcp::resolver resolver_;
  
  std::auto_ptr<asio::ip::tcp::socket> p_s_;

  int port_;

  boost::function<void (std::auto_ptr<asio::ip::tcp::socket>) > connect_handler_;
};

#endif
