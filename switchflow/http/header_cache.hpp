//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (c) Christopher Baus. All Rights Reserved
//
#ifndef SF_HEADER_CACHE_HPP
#define SF_HEADER_CACHE_HPP

#include <list>

#include "header_buffer.hpp"

namespace switchflow{
namespace http{
    
  class header_cache
  {
  public:
    header_cache(unsigned int num_headers,
                 unsigned int max_name_length,
                 unsigned int max_value_length);
    virtual ~header_cache();
    bool empty();
    void alloc_header(std::list<header_buffer>& header_list);
    void release_headers(std::list<header_buffer>& header_list);
    size_t num_available_headers();
  private:
    std::list<header_buffer> headers_;
  };
  
} //namespace http
} //namespace switchflow

#endif // HEADER_CACHE_HPP
