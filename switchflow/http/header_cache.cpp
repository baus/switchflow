//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include "header_cache.hpp"
#include <util/logger.hpp>

namespace http{

header_cache::header_cache(unsigned int num_headers,
                           unsigned int max_name_length,
                           unsigned int max_value_length):
  headers_(num_headers, header_buffer(max_name_length,
                                      max_value_length))
{
  
}

bool header_cache::empty()
{
  return headers_.empty();
}

void header_cache::alloc_header(std::list<header_buffer>& header_list)
{
  CHECK_CONDITION(!empty(), "empty header cache");
  //
  // Can I use end here or do I need --end() ?
  headers_.begin()->reset();
  header_list.splice(header_list.end(), headers_, headers_.begin());
  
}

void header_cache::release_headers(std::list<header_buffer>& header_list)
{
  CHECK_CONDITION(!empty(), "empty header cache");
  headers_.splice(headers_.end(),
                  header_list);
  
}

header_cache::~header_cache()
{
}

size_t header_cache::num_available_headers()
{
  return headers_.size();
}

}
