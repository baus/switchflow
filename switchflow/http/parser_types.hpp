//
// Copyright 2003-2007 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_HTTP_PARSER_TYPES_HPP
#define SF_HTTP_PARSER_TYPES_HPP

#include <asio.hpp>

namespace switchflow{
namespace http{


namespace parse{
enum status
{
  // Completely and successfully parsed component
  COMPLETE,

  // Incompletely yet successfully parsed component.
  // Need to read more data to continue parsing
  INCOMPLETE,

  // Was unable to complete processing component as
  // writing would block.  Need to wait for write
  // notification.
  PAUSE,
  
  // The component being parsed is invalid
  INVALID,
  
  // The component data is valid, but would overflow the component's buffer
  DATAOVERFLOW
};
};

struct parse_result
{
    parse::status status;
    asio::const_buffer buffer;
};

namespace receive{
enum status
{
    // Succesfully set and processed data
    SUCCESS,

    // The process must be paused for pending I/O
    PAUSE,

    // The component being parsed is invalid
    INVALID,
  
    // The component data is valid, but would overflow the component's buffer
    DATAOVERFLOW

};
};

enum buffer_status
{
    // The buffer contains the remainder of the component
    COMPLETE,

    // The buffer contains a sub amount of the component
    INCOMPLETE
};

parse::status convert_to_parse_status(receive::status r_status, buffer_status buf_status)
{
    if(buf_status == COMPLETE && r_status == receive::SUCCESS){
        return parse::COMPLETE;
    }
    else if(buf_status == INCOMPLETE && r_status == receive::SUCCESS){
        return parse::INCOMPLETE;
    }
    else if(r_status == receive::PAUSE){
        return parse::PAUSE;
    }
    else if(r_status == receive::INVALID){
        return parse::INVALID;
    }
    else if(r_status == receive::DATAOVERFLOW){
        return parse::DATAOVERFLOW;
    }
    return parse::INVALID;
}


} // namespace http
} // namespace switchflow

#endif 
