//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <stdlib.h>
#include <netdb.h>
#include <boost/cast.hpp>

#include <util/logger.hpp>
#include <http/static_strings.hpp>
#include <http/request_buffer_wrapper.hpp>
#include <http/response_buffer_wrapper.hpp>

#include "http_proxy_stream_handler.hpp"
#include "combined_log_record.hpp"
#include "pipeline_data.hpp"
#include <string>

http_proxy_stream_handler::http_proxy_stream_handler(switchflow::http::header_cache* cache,
                                                     const std::map<std::string, host_rules>& rules,
                                                     switchflow::http::STREAM_TYPE stream_type,
                                                     unsigned int max_start_line1_length, 
                                                     unsigned int max_start_line2_length, 
                                                     unsigned int max_start_line3_length,
                                                     unsigned int num_headers,
                                                     unsigned int max_header_name_length,
                                                     unsigned int max_header_value_length,
                                                     access_log* p_access_log,                                               
                                                     i_request_postprocessor* p_postprocessor):
  cache_(cache),
  host_rules_(rules),
  stream_type_(stream_type),
  header_parser_(&header_handler_),
  body_parser_(this),
  current_field_(0),
  current_dump_header_(0),
  max_start_line1_length_(max_start_line1_length),
  max_start_line2_length_(max_start_line2_length),
  max_start_line3_length_(max_start_line3_length),
  num_headers_(num_headers),
  max_header_name_length_(max_header_name_length),
  max_header_value_length_(max_header_value_length),
  message_state_(START_MESSAGE),
  push_header_state_(START_LINE_TOKEN1),
  chunk_size_(switchflow::http::CHUNK_SIZE_LENGTH + 2), // this allows room for the trailing CRLF
  p_access_log_(p_access_log),
  current_pipeline_data_(0),
  endline_buf_(&switchflow::http::strings_.endline_),
  response_(cache,
            max_start_line1_length, 
            max_start_line2_length, 
            max_start_line3_length,
            num_headers),
  header_handler_(response_, stream_type),
  request_postprocessor_(p_postprocessor)
{
}

http_proxy_stream_handler* http_proxy_stream_handler::clone()
{
  return new http_proxy_stream_handler( cache_,
                                     host_rules_,
                                     stream_type_,
                                     max_start_line1_length_,
                                     max_start_line2_length_,
                                     max_start_line3_length_,
                                     num_headers_,
                                     max_header_name_length_,
                                     max_header_value_length_,
                                     p_access_log_,
                                     request_postprocessor_);
}

socketlib::STATUS http_proxy_stream_handler::process_data(read_write_buffer& buf)
{
  //
  // This is a hack.  The proxylib is basically requesting a flush here.
  // If we are still buffering headers, we can't flush, so return
  // complete.  Probably should add a flush function to the API.
  if(buf.get_working_length() == 0 &&
     (message_state_ == PARSE_HEADER)){
    return socketlib::COMPLETE;
  }
  for(;;){
    switch(message_state_){
      case START_MESSAGE:
        if(!set_current_pipeline_data()){
#warning not sure about this return value
          if(stream_type_ == switchflow::http::REQUEST){
            return socketlib::INCOMPLETE;
          }
          else{
            return socketlib::COMPLETE;
          }
        }
        if(current_pipeline_data_->process_type_ == pipeline_data::DENY){
          error_response_.reset();
          header_writer_.reset(error_response_.get_message_buffer(),
                               *get_proxy_stream_interface()->get_dest());
          message_state_ = PUSH_HEADER;
        }
        else{
          message_state_ = PARSE_HEADER;
        }
        break;
        
      case PARSE_HEADER:  
        switchflow::http::STATUS parse_status;
        parse_status = header_parser_.parse_headers(buf);
        if(parse_status == switchflow::http::COMPLETE){
          message_state_ = PUSH_HEADER;
	  update_header_host_and_path();
          header_writer_.reset(header_handler_.get_message_buffer(),
                               *get_proxy_stream_interface()->get_dest());
          if(request_postprocessor_ && stream_type_ == switchflow::http::REQUEST){
            if(request_postprocessor_->process_request(
                 header_handler_.get_message_buffer())){
              log_request();
              break;
            }
            else{
              parse_status = switchflow::http::INVALID;
            }
          }
          else{
            log_request();
            break;
          }
          break;
        }
        if(parse_status == switchflow::http::INVALID || parse_status == switchflow::http::DATAOVERFLOW){
          if(stream_type_ == switchflow::http::REQUEST){
            current_pipeline_data_->process_type_ = pipeline_data::DENY;
            current_pipeline_data_->request_complete_ = true;
            log_info("invalid request");
          }
          else{
            log_info("invalid response");
          }
          return socketlib::DENY;
        }
        return socketlib::COMPLETE;

      case PUSH_HEADER:
        socketlib::STATUS push_status;
        push_status = header_writer_.write_header();
        if(push_status == socketlib::COMPLETE){
          message_state_ = PARSE_BODY;
          current_pipeline_data_->request_complete_ = true;
          if(current_pipeline_data_->process_type_ == pipeline_data::DENY){
            //
            // Force the end of connection.
            return socketlib::SRC_CLOSED;
          }
          else if(stream_type_ == switchflow::http::RESPONSE && 
             current_pipeline_data_->process_type_ == pipeline_data::HEAD){
            // 
            // If the request was a HEAD request, override 
            // what the response headers claim that the body encoding
            // is and set the body encoding to none.  This shit
            // drives me nuts about the HTTP spec.
            body_parser_.reset(switchflow::http::NONE, 0);
          }
          else{
            //
            // Just do the sensible thing, and get the body encoding
            // from the headers.
            body_parser_.reset(header_handler_.get_body_encoding(), 
                               header_handler_.get_message_size());
          }
          break;
        }
        return push_status;

      case PARSE_BODY:
        parse_status = body_parser_.parse_body(buf);
        if(parse_status == switchflow::http::COMPLETE){
          get_proxy_stream_interface()->flush();
          complete_message();
          break;
        }
        if(parse_status == switchflow::http::INVALID ||
           parse_status == switchflow::http::DATAOVERFLOW){
          return socketlib::DENY;
        }
        //
        // This next state was causing a subtle edge-triggered state jam problem.  The problem
        // here is that INCOMPLETE can mean two different things.
        //
        // 1) The body isn't completed in the buffer, and I need to read again.
        // 2) The upstream server isn't ready to write, and I need a write event.
        //
        // What was happening is in the first case I am returning
        // incomplete here, which causing the proxy_stream handler to wait
        // for another event, but there is data left to read in the socket,
        // and since we are edge-triggered we don't get notified again, hence
        // the state-machine jams.
        //
        // The problem was fixed by adding the second WRITE_INCOMPLETE code.
        if(parse_status == switchflow::http::INCOMPLETE){
          return socketlib::COMPLETE;
        }
        if(parse_status == switchflow::http::WRITE_INCOMPLETE){
          return socketlib::INCOMPLETE;
        }
        if(parse_status == switchflow::http::IOFAILURE){
          return socketlib::DEST_CLOSED;
        }
        CHECK_CONDITION_VAL(false, "unknown return value from parse_body", parse_status);
        break;
      default:
        CHECK_CONDITION_VAL(false, "unknown state in process_data", message_state_);
    }
  }
  //
  // Should return from inside loop
  CHECK_CONDITION(false, "fell out process_data loop");
}

switchflow::http::STATUS http_proxy_stream_handler::set_body(read_write_buffer& body, bool b_complete)
{
  //
  //
  socketlib::STATUS return_val = get_proxy_stream_interface()->forward(body);

  return convert_proxy_status(return_val);
}

void http_proxy_stream_handler::complete_message()
{
  clear_current_pipeline_data();
  initialize_state();
}

void http_proxy_stream_handler::reset()
{
  get_proxy_stream_interface()->get_pipeline_data_queue()->empty_queue();
  initialize_state();
}

void http_proxy_stream_handler::initialize_state()
{
  current_field_ = 0;
  current_dump_header_ = 0;
  message_state_ = START_MESSAGE;
  push_header_state_ = START_LINE_TOKEN1;
  header_parser_.reset();
  body_parser_.reset(switchflow::http::NONE, 0);
  chunk_size_.reset();
  header_handler_.reset();
  
}


void http_proxy_stream_handler::set_body_encoding(switchflow::http::BODY_ENCODING body_encoding)
{
  //
  // This is a no-op as we already know what the body_encoding is.
  // Not all i_http_body_receivers may have that luxury.
  //
}


void http_proxy_stream_handler::set_chunk_size(unsigned int chunk_size)
{
  int chunk_size_chars = snprintf((char*)(&chunk_size_.get_raw_buffer()[0]), chunk_size_.get_physical_length(), "%X", chunk_size);
  chunk_size_.set_working_length(chunk_size_chars + 2);
  chunk_size_[chunk_size_chars] = switchflow::http::CR;
  chunk_size_[chunk_size_chars + 1] = switchflow::http::LF;
  
  endline_buf_.set_write_position(0);
}

switchflow::http::STATUS http_proxy_stream_handler::forward_chunk_trailer()
{
  socketlib::STATUS return_val = get_proxy_stream_interface()->forward(endline_buf_);
  return convert_proxy_status(return_val);
}

switchflow::http::STATUS http_proxy_stream_handler::forward_chunk_size()
{
  socketlib::STATUS return_val = get_proxy_stream_interface()->forward(chunk_size_);
  return convert_proxy_status(return_val);
}


void http_proxy_stream_handler::log_request()
{
  //
  // Don't bother to do anything if the log file
  // isn't open.  
  //
  if(!p_access_log_->is_open()){
    return;
  }
  
  if(stream_type_ == switchflow::http::REQUEST){
    current_pipeline_data_->logrecord_.reset();
    copy_request_data_to_log(header_handler_.get_message_buffer(),
                         current_pipeline_data_->logrecord_);
  }
  else if(stream_type_ == switchflow::http::RESPONSE){

    copy_response_data_to_log(header_handler_.get_message_buffer(),
                          current_pipeline_data_->logrecord_);
    
    p_access_log_->log_access(current_pipeline_data_->logrecord_);
    
  }
  else{
    CHECK_CONDITION(false, "invalid http socket request");
  }
}
 
void http_proxy_stream_handler::copy_request_data_to_log(switchflow::http::message_buffer& message_buffer,
                                                  combined_log_record& log_record)
{
  switchflow::http::request_buffer_wrapper buffer_wrapper(message_buffer);
  
  log_record.remote_ip = get_proxy_stream_interface()->get_src_address();
  
  buffer_wrapper.get_method().append_to_string(log_record.requestline);
  log_record.requestline += ' ';
  buffer_wrapper.get_uri().append_to_string(log_record.requestline);
  log_record.requestline += ' ';
  buffer_wrapper.get_http_version_buffer().append_to_string(log_record.requestline);
  
  unsigned int header_index;
  if(buffer_wrapper.get_header_with_name_index("referer", header_index)){ 
    buffer_wrapper.get_field_value_n(header_index).append_to_string(log_record.referer);
  }
  
  if(buffer_wrapper.get_header_with_name_index("user-agent", header_index)){ 
    buffer_wrapper.get_field_value_n(header_index).append_to_string(log_record.user_agent);
  }
}

void http_proxy_stream_handler::copy_response_data_to_log(switchflow::http::message_buffer& message_buffer,
                                                          combined_log_record& log_record)
{
  
  switchflow::http::response_buffer_wrapper buffer_wrapper(message_buffer);
  buffer_wrapper.get_status_code().append_to_string(log_record.status);
}

void http_proxy_stream_handler::update_header_host_and_path()
{
  switchflow::http::message_buffer& buffer = header_handler_.get_message_buffer();

  unsigned int hostname_index;
  if(!buffer.get_header_index_by_name("Host", hostname_index)){
    return;
  }

  std::string hostname;
  buffer.get_field_value(hostname_index).append_to_string(hostname);

  std::map<std::string, host_rules>::const_iterator it;

  it = host_rules_.find(hostname);
  if(it == host_rules_.end()){
    return;
  }
  
  switchflow::http::request_buffer_wrapper request_wrapper(header_handler_.get_message_buffer());

  std::string request_path;
  request_wrapper.get_uri().append_to_string(request_path);
  
  const httplib::url* p_forward_url = it->second.find_forward_url(request_path.c_str());
  if(p_forward_url == NULL){
    const httplib::url& forward_url = it->second.default_forward_url();
    std::string base_path = forward_url.path;
    buffer.get_status_line_2().append_to_string(base_path);
    buffer.get_status_line_2().append_from_string(base_path.c_str());
    if(!it->second.preserve_host()){
      buffer.get_field_value(hostname_index).append_from_string(forward_url.hostname.c_str());
    }
  }
  else{
    buffer.get_status_line_2().reset();
    buffer.get_status_line_2().append_from_string(p_forward_url->path.c_str());
    if(!it->second.preserve_host()){
      buffer.get_field_value(hostname_index).append_from_string(p_forward_url->hostname.c_str());
    }
  }
}


proxylib::i_proxy_stream_handler::FORWARD_ADDRESS_STATUS http_proxy_stream_handler::get_forward_address_status()
{
  switchflow::http::message_buffer& buffer = header_handler_.get_message_buffer();
  
  if(stream_type_ != switchflow::http::REQUEST){
    return proxylib::i_proxy_stream_handler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }

  switchflow::http::request_buffer_wrapper request_wrapper(header_handler_.get_message_buffer());
  if(request_wrapper.get_http_version() == switchflow::http::request_buffer_wrapper::HTTP1){
    return proxylib::i_proxy_stream_handler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }
  
  unsigned int hostname_index;
  if(!buffer.get_header_index_by_name("Host", hostname_index)){
    return proxylib::i_proxy_stream_handler::FORWARD_ADDRESS_NOT_YET_AVAILABLE;
  }

  std::string hostname;
  buffer.get_field_value(hostname_index).append_to_string(hostname);

  std::map<std::string, host_rules>::const_iterator it;

  it = host_rules_.find(hostname);
  if(it == host_rules_.end()){
    return proxylib::i_proxy_stream_handler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }

  std::string path;
  request_wrapper.get_uri().append_to_string(path);
  const httplib::url* p_forward_url = it->second.find_forward_url(path.c_str());
  if(p_forward_url == NULL){
    const httplib::url& forward_url = it->second.default_forward_url();
    forward_address_ = *forward_url.endpoint.data();
    
  }
  else{
    forward_address_ = *p_forward_url->endpoint.data();
    
  }
  return proxylib::i_proxy_stream_handler::FORWARD_ADDRESS_AVAILABLE;

#ifdef NOTUSINGJITCONNECT
  return proxylib::i_proxy_stream_handler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
#endif  
  
}


sockaddr& http_proxy_stream_handler::get_forward_address_from_stream()
{
  return forward_address_;
}


bool http_proxy_stream_handler::set_current_pipeline_data()
{
  if(stream_type_ == switchflow::http::REQUEST){
    if(get_proxy_stream_interface()->get_pipeline_data_queue()->queue_full()){
      return false;
    }
    current_pipeline_data_ =
      boost::polymorphic_downcast<pipeline_data*>(get_proxy_stream_interface()->get_pipeline_data_queue()->queue_element());
  }
  else if(stream_type_ == switchflow::http::RESPONSE){
    if(get_proxy_stream_interface()->get_pipeline_data_queue()->queue_empty()){
      return false;
    }
    else{
      current_pipeline_data_ =
      boost::polymorphic_downcast<pipeline_data*>(get_proxy_stream_interface()->get_pipeline_data_queue()->front());
      if(!current_pipeline_data_->request_complete_){
        return false;
      }
      get_proxy_stream_interface()->get_pipeline_data_queue()->dequeue_element();
    }
  }
  else{
    CHECK_CONDITION(false, "invalid http request type");
  }
  return true;
}

void http_proxy_stream_handler::clear_current_pipeline_data()
{
  current_pipeline_data_ = 0;
}

switchflow::http::STATUS http_proxy_stream_handler::convert_proxy_status(socketlib::STATUS status)
{
  if(status == socketlib::DEST_CLOSED){
    return switchflow::http::IOFAILURE;
  }
  else if(status == socketlib::COMPLETE){
    return switchflow::http::COMPLETE;
  }
  else if(status == socketlib::INCOMPLETE){
    return switchflow::http::INCOMPLETE;
  }
  else if(status == socketlib::SRC_CLOSED){
    return switchflow::http::IOFAILURE;
  }
  else if(status == socketlib::DENY){
    return switchflow::http::INVALID;
  }
  else{
    CHECK_CONDITION_VAL(false, "invalid status", status);
  }
  //
  // return something to avoid warning.
  //
  return switchflow::http::INVALID;
}
