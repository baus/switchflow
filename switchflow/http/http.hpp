//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SSD_HTTPLIB_HPP
#define SSD_HTTPLIB_HPP

#include <set>

#include <util/read_write_buffer.hpp>

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

/// Internal maximum for a field length.  Client's can
/// reject fields of a shorter length, but the length
/// can not exceed this internal max.
///
extern const unsigned int MAX_FIELD_LENGTH;

extern std::set<char> s_spaceDelimiters;
extern std::set<char> s_separators;
extern std::set<char> s_fieldDelimiters;
extern std::set<char> s_endlineDelimiters;
extern std::set<char> s_lfDelimiters;
extern std::set<char> s_whiteSpace;
extern std::set<char> s_spaceEndlineDelimiters;

void init();

class line_parser
{
public:
  enum LINE_END_OPTION{
    ALLOW_ONLY_LF,
    REQUIRE_CRLF
  };
  
  void reset(LINE_END_OPTION option, unsigned int max_length);
  STATUS parse_line(read_write_buffer& buffer,
                    unsigned int beginOffset,
                    unsigned int& endOffset);
  
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

STATUS parseNLengthBuffer(read_write_buffer& buffer,
                          unsigned int& currentLength,
                          unsigned int maxLength);

STATUS parseChar(read_write_buffer& buffer, char c);

STATUS parseChar(read_write_buffer& buffer,
                 unsigned int beginOffset,
                 unsigned int& endOffset,
                 char c);

STATUS parseToken(read_write_buffer& buffer,
                  unsigned int beginOffset,
                  unsigned int& endOffset,
                  unsigned int& currentLength,
                  unsigned int maxLength,
                  std::set<char>& delimiters);

bool isCTLChar(char c);

STATUS parseEqual(read_write_buffer& buffer,
                  unsigned int beginOffset,
                  unsigned int& endOffset,
                  unsigned int& currentLength,
                  unsigned int maxLength,
                  std::set<char>& compareChars);

} // namespace httplib

#endif // include guard
