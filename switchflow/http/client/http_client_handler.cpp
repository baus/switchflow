//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <fcntl.h>
#include <semaphore.h>

#include <string>
#include <iostream>
#include <memory>

#include <openssl/sha.h>
#include <boost/scoped_ptr.hpp>

#include <util/logger.hpp>
#include <event/poller.hpp>

#include <http/body_parser.hpp>
#include <http/request_buffer_wrapper.hpp>
#include <http/static_strings.hpp>

#include <client/client_handler.hpp>

#include <http/client/i_http_client.hpp>

#include "http_client_handler.hpp"


http_client_handler::http_client_handler(std::auto_ptr<i_http_client> p_http_client,
                                         switchflow::http::header_cache& cache):
  p_http_client_(p_http_client),
  endline_buf_(&switchflow::http::strings_.endline_),
  header_handler_(p_http_client->get_response(), switchflow::http::RESPONSE),
  header_parser_(&header_handler_),
  body_parser_(this)
{
  message_state_ = PUSH_HEADER;
  endline_buf_.reset();
  header_handler_.reset();
  header_parser_.reset();
}

http_client_handler::http_client_handler(const http_client_handler& rhs):
  endline_buf_(&switchflow::http::strings_.endline_),
  header_handler_(rhs.header_handler_),
  header_parser_(rhs.header_parser_),
  body_parser_(this)
{
  CHECK_CONDITION(false, "http_client_handler Copy construction not implemented");
}


http_client_handler::~http_client_handler()
{
}

void http_client_handler::connect(socketlib::connection& conn)
{
  header_writer_.reset(p_http_client_->get_request(), conn);
}

void http_client_handler::shutdown()
{
  header_handler_.reset();
  
  p_http_client_->shutdown();
}



socketlib::STATUS http_client_handler::handle_stream(socketlib::connection& socket)
{
  //
  // Handle stream should return complete if the
  // buffer is complete.
  socketlib::STATUS status;
  switchflow::http::STATUS parse_status;

  for(;;){
    switch(message_state_){
      case PUSH_HEADER:
        status = header_writer_.write_header();
        if(status == socketlib::COMPLETE){
          message_state_ = PUSH_BODY;
          break;
        }
        if(status == socketlib::INCOMPLETE){
          return socketlib::WRITE_INCOMPLETE;
        }
        p_http_client_->send_header_failed();
        return status;
        
      case PUSH_BODY:
        status = socket.non_blocking_write(endline_buf_);
        if(status == socketlib::COMPLETE){
          message_state_ = PARSE_RESPONSE_HEADER;
          break;
        }
        if(status == socketlib::INCOMPLETE){
          return socketlib::WRITE_INCOMPLETE;
        }
        p_http_client_->send_body_failed();
        return status;
        
      case PARSE_RESPONSE_HEADER:
        parse_status = header_parser_.parse_headers(socket.read_buffer(),
                                                    switchflow::http::header_parser::LOOSE);
        if(parse_status == switchflow::http::COMPLETE){
          message_state_ = PARSE_BODY;
          body_parser_.reset(header_handler_.get_body_encoding(), 
                             header_handler_.get_message_size());
          p_http_client_->headers_complete();   
          break;
        }
        if(parse_status == switchflow::http::INVALID || parse_status == http::DATAOVERFLOW){
          p_http_client_->invalid_header();   
          return socketlib::DENY;
        }
        if(!socket.open_for_read()){
          p_http_client_->invalid_header();
          return socketlib::DENY;
        }
        return socketlib::READ_INCOMPLETE;
        
      case PARSE_BODY:
      {
        parse_status = body_parser_.parse_body(socket.read_buffer());
        if(parse_status == http::INVALID || parse_status == http::DATAOVERFLOW){
          p_http_client_->invalid_body();
          return socketlib::DENY;
        }
        if(parse_status == http::INCOMPLETE){
          if(!socket.open_for_read()){
            if(body_parser_.encoding() == http::END_CONNECTION){
              //
              // This marks the end of the connection for HTTP 0.9 style connections.
              // It is ugly to do this here, but the body parser doesn't know when
              // the connection has been terminated.
              read_write_buffer temp;
              p_http_client_->handle_body(temp, true);
              p_http_client_->request_complete();
              return socketlib::COMPLETE;
            }
            p_http_client_->invalid_body();
            return socketlib::DENY;
          }
          return socketlib::READ_INCOMPLETE;
        }
        p_http_client_->request_complete();              
        return socketlib::COMPLETE;
      }
      
      default:
        CHECK_CONDITION(false, "invalid message state");
    }
  }
}

http::STATUS http_client_handler::set_body(read_write_buffer& body, bool b_complete)
{
  return p_http_client_->handle_body(body, b_complete);
}


void http_client_handler::set_body_encoding(http::BODY_ENCODING body_encoding)
{
}


void http_client_handler::set_chunk_size(unsigned int chunk_size)
{
}

http::STATUS http_client_handler::forward_chunk_trailer()
{
  return http::COMPLETE;
}

http::STATUS http_client_handler::forward_chunk_size()
{
  return http::COMPLETE;
}


bool http_client_handler::get_header_value(const char* header_name, std::string& header_value)
{
  unsigned int index_of_header;

  if(header_handler_
     .get_message_buffer()
     .get_header_index_by_name(header_name, index_of_header))
  {
    header_value = "";
    header_handler_
      .get_message_buffer()
      .get_field_value(index_of_header)
      .append_to_string(header_value);
    return true;
    
  }
  return false;
}


void http_client_handler::timeout()
{
  p_http_client_->timeout();
}

void http_client_handler::connect_failed()
{
  p_http_client_->connect_failed();
}

void http_client_handler::dns_failed()
{
  p_http_client_->dns_failed();
}

