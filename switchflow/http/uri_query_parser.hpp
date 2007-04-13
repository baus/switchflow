//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_URI_QUERY_PARSER_HPP
#define SF_URI_QUERY_PARSER_HPP

#include <asio.hpp>

#include "http.hpp" 

namespace switchflow{
namespace http{

class i_uri_query_receiver;

class uri_query_parser
{
 public:
  uri_query_parser(i_uri_query_receiver& receiver,
		   size_t max_key_length,
                   size_t max_value_length);


  std::pair<STATUS, asio::const_buffer> parse_uri_query(asio::const_buffer buffer);

  void reset();
  
private:

};

} //namespace httplib
} //namespace switchflow

#endif

