//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <http/message_buffer.hpp>
#include <http/header_parser.hpp>
#include <http/header_handler.hpp>
#include <http/body_parser.hpp>

#include <util/read_write_buffer.hpp>

#include "tcp_connector.hpp"
#include "i_connect_handler.hpp"
#include "i_http_connection_handler.hpp"

class stream_socket_connection_state
{
public:
  stream_socket_connection_state():state_(DISCONNECTED){}
  
  void connect()
  {
    if(DISCONNECTED == state_){
      state_ = CONNECTED_FOR_READ_AND_WRITE;
    }
    //
    // ignore attempts to reconnect socket
  }
  
  void shutdown_read()
  {
    if(CONNECTED_FOR_READ == state_){
      state_ = DISCONNECTED;
    }
    else if(CONNECTED_FOR_READ_AND_WRITE == state_){
      state_ = CONNECTED_FOR_WRITE;
    }
    //
    // ignore re-shutting down.
  }
  
  void shutdown_write()
  {
    if(CONNECTED_FOR_WRITE == state_){
      state_ = DISCONNECTED;
    }
    else if(CONNECTED_FOR_READ_AND_WRITE == state_){
      state_ = CONNECTED_FOR_READ;
    }
    //
    // ignore re-shutting down.
  }
  
  void disconnect()
  {
    state_ = DISCONNECTED;
  }
  
  //
  // Most apps only care if they can read or write
  // to a socket, but all states can be determined.
  bool connected_for_read()
  {
    return CONNECTED_FOR_READ || CONNECTED_FOR_READ_AND_WRITE;
  }
  bool connected_for_write()
  {
    return CONNECTED_FOR_WRITE || CONNECTED_FOR_READ_AND_WRITE;
  }

private:
  enum SOCKET_STATE
  {
    DISCONNECTED,
    CONNECTED_FOR_READ,
    CONNECTED_FOR_WRITE,
    CONNECTED_FOR_READ_AND_WRITE
  };

  SOCKET_STATE state_;
};

class http_client: private boost::noncopyable, private http::i_body_receiver
{
public:
  http_client(asio::io_service& d,
              const char* ipv4_tcp_address,
              int ipv4_tcp_port,
              http::header_cache& cache,
              i_connect_error_handler& connect_errors,
              i_http_connection_handler& http_handler);

  void make_request(http::message_buffer& message);

private:
  
  enum MESSAGE_STATE
  {
    PARSE_RESPONSE_HEADER,
    PARSE_BODY
  };

  enum PARSE_RESULT
  {
    INCOMPLETE,
    COMPLETE,
    DENY
  };
  
  friend class tcp_connector;
  
  void connect_handler(std::auto_ptr<asio::ip::tcp::socket> p_socket);

  void write_request(std::list<asio::const_buffer>& request);  

  void request_write_handler(const asio::error& error);

  void response_read_handler(const asio::error& error, 
                             std::size_t bytes_transferred);

  void set_body_encoding(http::BODY_ENCODING body_encoding);

  PARSE_RESULT parse_response();

  PARSE_RESULT parse_response_headers();

  PARSE_RESULT parse_response_body();

  http::STATUS set_body(read_write_buffer& buffer, bool complete);

  void set_chunk_size(unsigned int chunk_size);

  http::STATUS forward_chunk_size();

  http::STATUS forward_chunk_trailer();

  tcp_connector connector_;
  i_http_connection_handler& http_handler_;
  
  std::auto_ptr<asio::ip::tcp::socket> p_socket_;

  std::list< std::list<asio::const_buffer> > request_queue_; 

  read_write_buffer read_buffer_;

  http::message_buffer response_;
  http::header_handler header_handler_;
  http::HTTPHeaderParser header_parser_;
  http::BodyParser body_parser_;

  MESSAGE_STATE response_state_;

  stream_socket_connection_state socket_state_;
};

#endif
