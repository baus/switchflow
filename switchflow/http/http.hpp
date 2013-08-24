//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HTTP_HPP
#define SF_HTTP_HPP

#include <set>

#include <util/read_write_buffer.hpp>

namespace switchflow{
namespace http{

// Return Status codes used by HTTPParsers.
//

enum STATUS
{
  // Completely and successfully parsed component
  COMPLETE,
  // Incompletely yet successfully parsed component.
  // Need to read more data to continue parsing
  INCOMPLETE,
  // Was unable to complete processing component as
  // writing would block.  Need to wait for write
  // notification.
  WRITE_INCOMPLETE,
  // The component being parsed is invalid
  INVALID,
  // The component data is valid, but would overflow the component's buffer
  DATAOVERFLOW,
  // Failed to set the data to upstream handler.
  IOFAILURE
};

// How is the body encoded?
//
enum BODY_ENCODING
{
  // There is no body
  NONE,
  // uses chunked encoding
  CHUNKED,
  // the body ends at the end of the connection
  END_CONNECTION,
  // the body length is defined by the content-length header
  CONTENT_LENGTH
};

extern const int CR; 
extern const int LF;
extern const int SP;
extern const int HT;
extern const int DEL;  

/// My first thought was to make this configurable, then I realized
/// that it is really dependant on the integer size used to store the parsed
/// chunk size.  8 is the exact number of characters that can be use
/// to represent an unsigned 32 bit int.
///
extern const unsigned int CHUNK_SIZE_LENGTH;

/// Internal maximum for a field length.  Clients can
/// reject fields of a shorter length, but the length
/// cannot exceed this internal max.
///
extern const unsigned int MAX_FIELD_LENGTH;

extern std::set<char> s_space_delimiters;
extern std::set<char> s_separators;
extern std::set<char> s_field_delimiters;
extern std::set<char> s_endline_delimiters;
extern std::set<char> s_lf_delimiters;
extern std::set<char> s_white_space;
extern std::set<char> s_space_endline_delimiters;

void init();

  
struct http_parse_result
{
  STATUS status;
  boost::asio::const_buffer result_buffer;
  boost::asio::const_buffer remaining_buffer;
  char delimiter;
};

class old_line_parser
{
public:
  enum LINE_END_OPTION{
    ALLOW_ONLY_LF,
    REQUIRE_CRLF
  };
  
  void reset(LINE_END_OPTION option, unsigned int max_length);
  STATUS parse_line(read_write_buffer& buffer,
                    unsigned int begin_offset,
                    unsigned int& end_offset);
  
private:
  enum STATE{
    PARSE_LINE,
    PARSE_LF,
    PARSE_COMPLETE
  };
  unsigned int current_length_;
  unsigned int max_length_;
  STATE state_;
  LINE_END_OPTION line_end_option_;
  unsigned int begin_offset;
};


STATUS parse_n_length_buffer(read_write_buffer& buffer,
                             unsigned int& current_length,
                             unsigned int max_length);

STATUS parse_char(read_write_buffer& buffer, char c);


STATUS parse_char(read_write_buffer& buffer,
                  unsigned int begin_offset,
                  unsigned int& end_offset,
                  char c);


http_parse_result parse_token(boost::asio::const_buffer buffer,
                         size_t current_length,
                         size_t max_length,
                         const std::set<char>& delimiters);

std::pair<STATUS, boost::asio::const_buffer> parse_char(boost::asio::const_buffer buffer,
                                                        char c);


STATUS parse_token(read_write_buffer& buffer,
                   unsigned int begin_offset,
                   unsigned int& end_offset,
                   unsigned int& current_length,
                   unsigned int max_length,
                   std::set<char>& delimiters);

bool is_ctl_char(char c);


STATUS parse_equal(read_write_buffer& buffer,
                  unsigned int begin_offset,
                  unsigned int& end_offset,
                  unsigned int& current_length,
                  unsigned int max_length,
                  std::set<char>& compare_chars);


std::pair<STATUS, boost::asio::const_buffer> parse_equal(boost::asio::const_buffer buffer,
                                                         size_t max_length,
                                                         std::set<char>& compare_chars);

} // namespace http
} // namespace switchflow

#endif 
