//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/logger.hpp>

#include "uri_query_parser.hpp"
#include "i_uri_query_receiver.hpp"

namespace switchflow{
namespace http{

  
uri_query_parser::uri_query_parser(i_uri_query_receiver& receiver,
				   size_t max_key_length,
				   size_t max_key_value)
{
}

std::pair<STATUS, asio::const_buffer> uri_query_parser::parse_uri_query(asio::const_buffer buffer, bool end_of_buffer)
{
}

void uri_query_parser::reset()
{
}

}// namespace httplib
}// namespace switchflow
