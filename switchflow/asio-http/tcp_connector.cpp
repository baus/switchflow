//
// Copyright (c) Christopher Baus.  All Rights Reserved.
#include "tcp_connector.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string>
#include <memory>

#include <util/logger.hpp>
#include <event/poller.hpp>
#include <boost/bind.hpp>


tcp_connector::tcp_connector(asio::io_service& d,
                             asio::ip::tcp::endpoint endpoint,
               boost::function< void (std::auto_ptr<asio::ip::tcp::socket>) > connect_handler,
                             i_connect_error_handler& connect_error_handler):d_(d),
                                                                             endpoint_(endpoint),
                                                                             connect_handler_(connect_handler),
                                                                             connect_error_handler_(connect_error_handler),
                                                                             resolver_(d),
                                                                             p_s_(0)
                                                                             
  {
    connect();
  }

  tcp_connector::tcp_connector(asio::io_service& d,
                               const char* address,
                               int port,
                 boost::function< void (std::auto_ptr<asio::ip::tcp::socket>) > connect_handler,
                               i_connect_error_handler& connect_error_handler):
    d_(d),
    port_(port),
    connect_handler_(connect_handler),
    connect_error_handler_(connect_error_handler),
    resolver_(d_),
    p_s_(0)
{
  asio::ip::tcp::resolver::query query(address, "http");
    
  resolver_.async_resolve(query,
          boost::bind(&tcp_connector::resolve_handler, this,
                      asio::placeholders::error,
                      asio::placeholders::iterator));
  
  
  
  }


  void tcp_connector::connect_handler(const asio::error& error)
  {
    if(!error){
      connect_handler_(p_s_);
    }
    else{
      connect_error_handler_.connect_failed();
    }
  }


  
  void tcp_connector::resolve_handler(const asio::error& error,
                    asio::ip::tcp::resolver::iterator iter)
  {
  asio::ip::tcp::resolver::iterator end;
    if(!error && iter != end){
      //
      // should pass in crawl request here.
      endpoint_ = *iter;;
    endpoint_.port(port_);
      
    p_s_.reset(new asio::ip::tcp::socket(d_));
      connect();  
    }
    else{
      connect_error_handler_.dns_failed();
    }
  }


  void tcp_connector::connect()
  {
    p_s_->async_connect(endpoint_, boost::bind(&tcp_connector::connect_handler,
                                             this,
                                             asio::placeholders::error));
    
  }
