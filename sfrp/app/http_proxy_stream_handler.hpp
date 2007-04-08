//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SFRP_HTTP_PROXY_STREAM_HANDLER_H
#define SFRP_HTTP_PROXY_STREAM_HANDLER_H

#include <boost/function.hpp>

#include <util/read_write_buffer.hpp>
#include <http/http.hpp>
#include <http/header_parser.hpp>
#include <http/body_parser.hpp>
#include <http/i_body_receiver.hpp>
#include <http/header_handler.hpp>
#include <http/header_pusher.hpp>
#include <http/error_response.hpp>
#include <http/header_cache.hpp>

#include <proxylib/proxy_handler.hpp>
#include <http/message_buffer.hpp>
#include <proxylib/pipeline_data_queue.hpp>

#include "combined_log_record.hpp"
#include "access_log.hpp"
#include "pipeline_data.hpp"
#include "i_request_postprocessor.hpp"
#include "host_rules.hpp"
#include <string>

// This class is the glue between the low level proxylib and the httplib
// application layer.  It implements interfaces from both layers to provide
// the glue mechanism.  
//
// Events are posted from the proxy network layer, and then passed to http_header_parser.
// Parsed headers are then passed back, and sent to the recv side of the stream.
//
// A proxy stream is one way. It handles either a request line, or
// response line, but not both.
class http_proxy_stream_handler: public proxylib::i_proxy_stream_handler, 
                                 public switchflow::http::i_body_receiver
{
 public:
  
  http_proxy_stream_handler(switchflow::http::header_cache* cache,
                            const std::map<std::string, host_rules>& rules,
                            switchflow::http::STREAM_TYPE stream_type,
                            unsigned int max_start_line1_length, 
                            unsigned int max_start_line2_length, 
                            unsigned int max_start_line3_length,
                            unsigned int num_headers,
                            unsigned int max_header_name_length,
                            unsigned int max_header_value_length,
                            access_log* p_access_log,
                            i_request_postprocessor* p_postprocessor);

  virtual ~http_proxy_stream_handler(){}

  http_proxy_stream_handler* clone();

  
  socketlib::STATUS process_data(read_write_buffer& buf);
  
  void reset();

  unsigned int get_message_size();
  switchflow::http::BODY_ENCODING get_body_encoding();

  // i_body_receiver interface implementation
  switchflow::http::STATUS set_body(read_write_buffer& body, bool b_complete);
  void set_body_encoding(switchflow::http::BODY_ENCODING body_encoding);
  switchflow::http::STATUS forward_chunk_size();
  switchflow::http::STATUS forward_chunk_trailer();
  void set_chunk_size(unsigned int chunk_size);

  proxylib::i_proxy_stream_handler::FORWARD_ADDRESS_STATUS get_forward_address_status();
  sockaddr& get_forward_address_from_stream();

 private:
  http_proxy_stream_handler();  

  enum MESSAGE_STATE
  {
    START_MESSAGE,
    PARSE_HEADER,
    PUSH_HEADER,
    PARSE_BODY
  };
  
  //
  // The current state of pushing headers to the
  // next upstream HTTP processor.  We might return 
  // from any of these sub-states.
  enum PUSH_HEADER_STATE
  {
    START_LINE_TOKEN1,
    START_LINE_SEPERATOR1,
    START_LINE_TOKEN2,
    START_LINE_SEPERATOR2,
    START_LINE_TOKEN3,
    START_LINE_END,
    FIELD_NAME,
    FIELD_VALUE_SEPERATOR,
    FIELD_VALUE,
    FIELD_SEPERATOR,
    END_FIELDS
  };
  

  void log_request();

  void copy_request_data_to_log(switchflow::http::message_buffer& message_buffer,
                                combined_log_record& log_record);

  void copy_response_data_to_log(switchflow::http::message_buffer& message_buffer,
                                 combined_log_record& log_record);

  bool set_current_pipeline_data();
  void clear_current_pipeline_data();
  void complete_message();
  void initialize_state();
  switchflow::http::STATUS convert_proxy_status(socketlib::STATUS status);

  
  //
  // Helper function to retrieve which header buffer to push
  read_write_buffer& get_header_buffer_to_push(PUSH_HEADER_STATE header_state);

  switchflow::http::header_cache* cache_;
  const std::map<std::string, host_rules> host_rules_;
  
  //
  // Is this a request or response stream?
  switchflow::http::STREAM_TYPE stream_type_;

  switchflow::http::header_parser header_parser_;
  switchflow::http::body_parser body_parser_;
  
  unsigned int current_field_;
  unsigned int current_dump_header_;
  
  // these are stored off for the clone 
  //
  // message_buffer should probably make it easier to retract these
  unsigned int max_start_line1_length_;
  unsigned int max_start_line2_length_;
  unsigned int max_start_line3_length_;
  unsigned int num_headers_;
  unsigned int max_header_name_length_;
  unsigned int max_header_value_length_;

  //
  // This is the high level HTTP message handling state.  All three
  // states are broken down into sub-states that are a handled by separate 
  // parsing functions.
  //
  MESSAGE_STATE message_state_;
  
  //
  // The sub-state that occurs while in the PUSH_HEADER state for 
  // the HTTP message.  
  int push_header_state_;
  
  access_log* p_access_log_;

  sockaddr forward_address_;
  
  read_write_buffer chunk_size_;

  pipeline_data* current_pipeline_data_;

  read_write_buffer endline_buf_;

  switchflow::http::error_response error_response_;

  switchflow::http::header_writer header_writer_;

  switchflow::http::message_buffer response_; 

  switchflow::http::header_handler header_handler_;
  
  i_request_postprocessor* request_postprocessor_;
};

#endif // SFRP_HTTP_PROXY_STREAM_HANDLER_H
