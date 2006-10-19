#ifndef SSD_HEADERHANDLER_H
#define SSD_HEADERHANDLER_H

#include <socketlib/status.hpp>
#include <util/read_write_buffer.hpp>
//#include <proxylib/ProxyStreamInterface.h>
#include <http/i_header_receiver.hpp>

#include "message_buffer.hpp"
#include "header_cache.hpp"

namespace http
{

//
// It is required to parameterize the stream handler by type
// so the parser can determine the body encoding properly.
enum STREAM_TYPE
{
  REQUEST,
  RESPONSE
};

size_t max_method_length();

extern const unsigned int max_version_length;

enum METHOD_TYPES{
  GET,
  POST,
  HEAD,
  NUM_VALID_METHODS
};

class header_handler: public http::IHeaderReceiver
{
public:
  header_handler(message_buffer& response,
                 STREAM_TYPE stream_type);

  virtual ~header_handler();
  socketlib::STATUS push_header();
  void reset();

  http::STATUS startLineToken1(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);
  http::STATUS startLineToken2(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);
  http::STATUS startLineToken3(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);
  http::STATUS setFieldName(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);
  http::STATUS setFieldValue(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete);  

  http::STATUS endFields();
  
  http::BODY_ENCODING get_body_encoding();

  unsigned int get_message_size();
 
  message_buffer& get_message_buffer();
 
private:
 

  //
  // Scan the response return code for processing information.
  //
  // Required to determine how to process the message
  // body.  For instance if the return code is 304.
  //
  // Called after parsing the return code from a response, and setting
  void scan_return_code();

  void scan_field();

  http::message_buffer& message_buffer_;
  
  unsigned int current_field_;
  unsigned int message_size_;

  bool have_host_header_;

  //
  // This is used when receiving 304s.  In that case, even if the
  // headers say there is a body, just ignore that, and assume no body.
  bool ignore_body_encoding_headers_;

  //
  // How is the message body encoded?
  http::BODY_ENCODING body_encoding_;

  STREAM_TYPE stream_type_;

  void log_error(char* error);
};

}

#endif

