//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_HTTPPROXYSTREAMHANDLER_H
#define SSD_HTTPPROXYSTREAMHANDLER_H

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

#include <proxylib/ProxyHandler.h>
#include <http/message_buffer.hpp>
#include <proxylib/pipeline_data_queue.hpp>

#include "CombinedLogRecord.hpp"
#include "AccessLog.hpp"
#include "pipeline_data.hpp"
#include "i_request_postprocessor.hpp"
#include "host_map.hpp"

// This class is the glue between the low level proxylib and the httplib
// application layer.  It implements interfaces from both layers to provide
// the glue mechanism.  
//
// Events are posted from the proxy network layer, and then passed to HTTPHeaderParser.
// Parsed headers are then passed back, and sent to the recv side of the stream.
//
// A proxy stream is one way. It handles either a request line, or
// response line, but not both.
class HTTPProxyStreamHandler: public proxylib::IProxyStreamHandler, 
                              public http::i_body_receiver
{
 public:
  
  HTTPProxyStreamHandler(http::header_cache* cache,
                         const std::map<std::string, std::pair<httplib::URL, bool> >& host_map,
                         http::STREAM_TYPE streamType,
                         unsigned int maxStartLine1Length, 
                         unsigned int maxStartLine2Length, 
                         unsigned int maxStartLine3Length,
                         unsigned int numHeaders,
                         unsigned int maxHeaderNameLength,
                         unsigned int maxHeaderValueLength,
                         AccessLog* pAccessLog,
                         i_request_postprocessor* p_postprocessor);

  virtual ~HTTPProxyStreamHandler(){}

  HTTPProxyStreamHandler* clone();

  
  socketlib::STATUS processData(read_write_buffer& buf);
  
  void reset();

  unsigned int getMessageSize();
  http::BODY_ENCODING getBodyEncoding();

  // IBodyReceiver interface implementation
  http::STATUS set_body(read_write_buffer& body, bool bComplete);
  void set_body_encoding(http::BODY_ENCODING bodyEncoding);
  http::STATUS forward_chunk_size();
  http::STATUS forward_chunk_trailer();
  void set_chunk_size(unsigned int chunkSize);

  proxylib::IProxyStreamHandler::FORWARD_ADDRESS_STATUS getForwardAddressStatus();
  sockaddr& getForwardAddressFromStream();

 private:
  HTTPProxyStreamHandler();  

  enum MESSAGE_STATE
  {
    START_MESSAGE,
    PARSE_HEADER,
    PUSH_HEADER,
    PARSE_BODY
  };
  
  //
  // The current state of pushing headers to 
  // next up stream HTTP processor.  We might return 
  // from any of these sub states.
  enum PUSH_HEADER_STATE{
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
  

  void logRequest();

  void copyRequestDataToLog(http::message_buffer& messageBuffer,
                            CombinedLogRecord& logRecord);

  void copyResponseDataToLog(http::message_buffer& messageBuffer,
                             CombinedLogRecord& logRecord);

  bool set_current_pipeline_data();
  void clear_current_pipeline_data();
  void complete_message();
  void initialize_state();
  http::STATUS convert_proxy_status(socketlib::STATUS status);

  
  //
  // Helper function to retrieve which header buffer to push
  read_write_buffer& getHeaderBufferToPush(PUSH_HEADER_STATE headerState);

  //
  // Is this a request or response stream.
  http::STREAM_TYPE streamType_;
  
  //  httpproxylib::message_buffer messageBuffer_;

  http::HTTPHeaderParser headerParser_;
  http::BodyParser bodyParser_;
  
  unsigned int currentField_;
  unsigned int currentDumpHeader_;
  
  // these are stored off for the clone 
  //
  // message_buffer should probably make it easier to retract these
  unsigned int maxStartLine1Length_;
  unsigned int maxStartLine2Length_;
  unsigned int maxStartLine3Length_;
  unsigned int numHeaders_;
  unsigned int maxHeaderNameLength_;
  unsigned int maxHeaderValueLength_;

  //
  // This is the high level HTTP message handling state.  All three
  // states are broken down into sub-states that are a handled by separate 
  // parsing functions.
  //
  MESSAGE_STATE messageState_;
  
  //
  // The substate that occurs while in the PUSH_HEADER state for 
  // the HTTP message.  
  int pushHeaderState_;
  
  AccessLog* pAccessLog_;

  sockaddr forwardAddress_;
  
  read_write_buffer chunkSize_;

  pipeline_data* current_pipeline_data_;

  read_write_buffer endlineBuf_;

  http::error_response error_response_;

  http::header_pusher header_pusher_;

  http::message_buffer response_; 

  http::header_handler header_handler_;

  
  http::header_cache* cache_;

  const std::map<std::string, std::pair<httplib::URL, bool> >& host_map_;
  i_request_postprocessor* request_postprocessor_;
};

#endif
