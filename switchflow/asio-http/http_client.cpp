//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "http_client.hpp"
#include "boost/bind.hpp"
#include <memory>

http_client::http_client(asio::io_service& d,
                         const char* address,
                         int port,
                         http::header_cache& cache,
                         i_connect_error_handler& connect_errors,
                         i_http_connection_handler& http_handler):
  connector_(d,
             address,
             port,
             boost::bind(&http_client::connect_handler, this, _1),
             connect_errors),
  http_handler_(http_handler),
  read_buffer_(5000),
  response_(&cache, 15, 15, 15, 50),
  header_handler_(response_, http::RESPONSE),
  header_parser_(&header_handler_),
  body_parser_(this),
  response_state_(PARSE_RESPONSE_HEADER)
  {}

  void http_client::connect_handler(std::auto_ptr<asio::ip::tcp::socket> p_socket)
{
  p_socket_ = p_socket;
  socket_state_.connect();
  while(!request_queue_.empty()){
    write_request(request_queue_.front());
    request_queue_.pop_front();
  }
}

void http_client::make_request(http::message_buffer& message)
{
  if(p_socket_.get() == 0){
    request_queue_.push_back(message.get_const_buffers());
  }
  else{
    std::list<asio::const_buffer> buffers = message.get_const_buffers();
    write_request(buffers);
  }
} 


void http_client::write_request(std::list<asio::const_buffer>& request)
{
  asio::async_write(*p_socket_,
                    request,
                    boost::bind(&http_client::request_write_handler,
                                this,
                                asio::placeholders::error));

}


void http_client::request_write_handler(const asio::error& error)
{
  if(error){
    socket_state_.shutdown_write();
    http_handler_.send_failed();
    return;
  }
  
  response_state_ = PARSE_RESPONSE_HEADER;

  p_socket_->async_read_some(asio::buffer(&(read_buffer_.get_raw_buffer()[0]), 
                             read_buffer_.get_physical_length()),
  boost::bind(&http_client::response_read_handler,
                this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred)
   );


}


http_client::PARSE_RESULT http_client::parse_response()
{
  http_client::PARSE_RESULT status;
  for(;;){
  switch(response_state_){
    case PARSE_RESPONSE_HEADER:
      if(COMPLETE == (status = parse_response_headers()))
      {
        break;
      }
      return status;
    case PARSE_BODY:
      return parse_response_body();
    }
  }
}

void http_client::response_read_handler(const asio::error& error, std::size_t bytes_transferred)
{
  read_buffer_.set_working_length(bytes_transferred);
  
  if(error){
    socket_state_.shutdown_write();
  }
  else{
    while(COMPLETE == parse_response() /** && more responses pending **/);
    p_socket_->async_read_some(asio::buffer(&(read_buffer_.get_raw_buffer()[0]), 
                             read_buffer_.get_physical_length()),
                boost::bind(&http_client::response_read_handler,
                this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred)
   );
  }
  
}


http::STATUS http_client::set_body(read_write_buffer& buffer, bool b_complete)
{
  int start_position = buffer.get_write_position();
  int num_bytes_to_write = buffer.get_write_end_position() - start_position;
  if(num_bytes_to_write > 0){
    asio::mutable_buffer mut_buffer(&buffer[start_position], num_bytes_to_write);
    http_handler_.accept_peer_body(mut_buffer);
  }
  return http::COMPLETE;
}

void http_client::set_body_encoding(http::BODY_ENCODING body_encoding)
{
}

void http_client::set_chunk_size(unsigned int chunk_size)
{
}

http::STATUS http_client::forward_chunk_size()
{
  return http::COMPLETE;
}

http::STATUS http_client::forward_chunk_trailer()
{
  return http::COMPLETE;
}

http_client::PARSE_RESULT http_client::parse_response_headers()
{
  http::STATUS parse_status = header_parser_.parse_headers(read_buffer_,
                                                           http::http_header_parser::LOOSE);
  if(parse_status == http::COMPLETE){
    response_state_ = PARSE_BODY;
    body_parser_.reset(header_handler_.get_body_encoding(), 
                        header_handler_.get_message_size());
    http_handler_.headers_complete();   
    return COMPLETE;
  }
  if(parse_status == http::INVALID || parse_status == http::DATAOVERFLOW){
    http_handler_.invalid_peer_header();    
    return DENY;
    
  }
  if(!socket_state_.connected_for_read()){
    http_handler_.invalid_peer_header();
    return DENY;
  }
  return INCOMPLETE;  
}

http_client::PARSE_RESULT http_client::parse_response_body()
{
  http::STATUS parse_status = body_parser_.parse_body(read_buffer_);
  if(parse_status == http::INVALID || parse_status == http::DATAOVERFLOW){
    
    http_handler_.invalid_peer_body();
    return DENY;
  }
  if(parse_status == http::INCOMPLETE){
    if(!socket_state_.connected_for_read()){
      if(body_parser_.encoding() == http::END_CONNECTION){
        //
        // This marks the end of the connection for HTTP 0.9 style connections.
        // It is ugly to do this here, but the body parser doesn't know when
        // the connection has been terminated.
        http_handler_.request_complete();
        response_state_ = PARSE_RESPONSE_HEADER;
        return COMPLETE;
      }
      http_handler_.invalid_peer_body();
      return DENY;
    }
    return INCOMPLETE;
  }
  http_handler_.request_complete();
  response_state_ = PARSE_RESPONSE_HEADER;
  return COMPLETE;
}
