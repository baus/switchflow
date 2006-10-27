//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (c) Christopher Baus.  All Rights Reserved.
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
                                         http::header_cache& cache):
  p_http_client_(p_http_client),
  m_endlineBuf(&http::strings_.endline_),
  header_handler_(p_http_client->get_response(), http::RESPONSE),
  m_headerParser(&header_handler_),
  m_bodyParser(this)
{
  m_messageState = PUSH_HEADER;
  m_endlineBuf.reset();
  header_handler_.reset();
  m_headerParser.reset();
}

http_client_handler::http_client_handler(const http_client_handler& rhs):
  m_endlineBuf(&http::strings_.endline_),
  header_handler_(rhs.header_handler_),
  m_headerParser(rhs.m_headerParser),
  m_bodyParser(this)
{
  CHECK_CONDITION(false, "http_client_handler Copy construction not implemented");
}


http_client_handler::~http_client_handler()
{
}

void http_client_handler::connect(socketlib::connection& conn)
{
  header_pusher_.reset(p_http_client_->get_request(), conn);
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
  http::STATUS parseStatus;

  for(;;){
    switch(m_messageState){
      case PUSH_HEADER:
        status = header_pusher_.push_header();
        if(status == socketlib::COMPLETE){
          m_messageState = PUSH_BODY;
          break;
        }
        if(status == socketlib::INCOMPLETE){
          return socketlib::WRITE_INCOMPLETE;
        }
        p_http_client_->send_header_failed();
        return status;
        
      case PUSH_BODY:
        status = socket.non_blocking_write(m_endlineBuf);
        if(status == socketlib::COMPLETE){
          m_messageState = PARSE_RESPONSE_HEADER;
          break;
        }
        if(status == socketlib::INCOMPLETE){
          return socketlib::WRITE_INCOMPLETE;
        }
        p_http_client_->send_body_failed();
        return status;
        
      case PARSE_RESPONSE_HEADER:
        parseStatus = m_headerParser.parseHeaders(socket.readBuffer(),
                                                  http::HTTPHeaderParser::LOOSE);
        if(parseStatus == http::COMPLETE){
          m_messageState = PARSE_BODY;
          m_bodyParser.reset(header_handler_.get_body_encoding(), 
                             header_handler_.get_message_size());
          p_http_client_->headers_complete();   
          break;
        }
        if(parseStatus == http::INVALID || parseStatus == http::DATAOVERFLOW){
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
        parseStatus = m_bodyParser.parseBody(socket.readBuffer());
        if(parseStatus == http::INVALID || parseStatus == http::DATAOVERFLOW){
          p_http_client_->invalid_body();
          return socketlib::DENY;
        }
        if(parseStatus == http::INCOMPLETE){
          if(!socket.open_for_read()){
            if(m_bodyParser.encoding() == http::END_CONNECTION){
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
        assert(false);
    }
  }
}

http::STATUS http_client_handler::set_body(read_write_buffer& body, bool bComplete)
{
  return p_http_client_->handle_body(body, bComplete);
}


void http_client_handler::set_body_encoding(http::BODY_ENCODING bodyEncoding)
{
}


void http_client_handler::set_chunk_size(unsigned int chunkSize)
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
      .appendToString(header_value);
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

