//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <util/logger.hpp>
#include <cstring>
#include <string>

#include "message_buffer.hpp"

typedef std::vector<BYTE> raw_buffer;

namespace switchflow{
namespace http{
  
  const char* message_buffer::new_line = "\r\n";
  const char* message_buffer::field_sep = ": ";
  const char* message_buffer::space = " ";

  message_buffer::message_buffer(header_cache* cache,
                                 unsigned int max_start_line1Length, 
                                 unsigned int max_start_line2Length, 
                                 unsigned int max_start_line3Length,
                                 unsigned int max_num_headers):
    start_line1_(max_start_line1Length),
    start_line2_(max_start_line2Length), 
    start_line3_(max_start_line3Length), 
    max_fields_(max_num_headers),
    current_num_fields_(0),
    cache_(cache)
{
}
  

  message_buffer::message_buffer(raw_buffer* start_line1,
                                 raw_buffer* start_line2,
                                 raw_buffer* start_line3,
                                 std::list<header_buffer*>& fields):
    start_line1_(start_line1),
    start_line2_(start_line2),
    start_line3_(start_line3),
    current_num_fields_(0),
    max_fields_(0),
    cache_(0)
  {
    std::list<header_buffer*>::iterator current = fields.begin();
    for(;current != fields.end(); ++current){
      headers_.push_back(**current);
      ++current_num_fields_;
      ++max_fields_;
    }
  }
  

message_buffer::~message_buffer()
{
  if(cache_){
    cache_->release_headers(headers_);
  }
}

void message_buffer::reset()
{
  if(cache_){
    cache_->release_headers(headers_);
    current_num_fields_ = 0;
  }
  else{
    std::list<header_buffer>::iterator current = headers_.begin();
    for(;current != headers_.end();++current){
      current->reset();
    } 
  }
  start_line1_.reset();
  start_line2_.reset();
  start_line3_.reset();
}

bool message_buffer::append_to_status_line_1(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  if(start_line1_.fits_in_buffer(end - begin)){
    start_line1_.append_from_buffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_status_line_2(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  if(start_line2_.fits_in_buffer(end - begin)){
    start_line2_.append_from_buffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_status_line_3(raw_buffer::iterator begin, raw_buffer::iterator end)
{

  if(start_line3_.fits_in_buffer(end - begin)){
    start_line3_.append_from_buffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_name(unsigned int n, raw_buffer::iterator begin, raw_buffer::iterator end)
{
  header_buffer& header = get_header(n);
  if(header.get_name().fits_in_buffer(end - begin)){
    header.append_to_name(begin, end);
    return true;
  }
  return false;
}

header_buffer& message_buffer::get_header(int n)
{
  CHECK_CONDITION(n < current_num_fields_, "trying to get a non-existant header");
  
  std::list<header_buffer>::iterator current = headers_.begin();

  for(int i = 0; i < n; ++current, ++i);
  
  return *current;
}

bool message_buffer::append_to_value(unsigned int n, raw_buffer::iterator begin, raw_buffer::iterator end)
{
  
  header_buffer& header = get_header(n);
  if(header.get_value().fits_in_buffer(end - begin)){
    header.append_to_value(begin, end);
    return true;
  }
  return false;
}


read_write_buffer& message_buffer::get_status_line_1()
{
  return start_line1_;
}

read_write_buffer& message_buffer::get_status_line_2()
{
  return start_line2_;
}

read_write_buffer& message_buffer::get_status_line_3()
{
  return start_line3_;
}


read_write_buffer& message_buffer::get_field_name(unsigned int n)
{
  CHECK_CONDITION(n < current_num_fields_, "trying to get non-existant field");
  return get_header(n).get_name();
}

read_write_buffer& message_buffer::get_field_value(unsigned int n)
{
  CHECK_CONDITION(n < current_num_fields_, "trying to get value of non-existant field");
  return get_header(n).get_value();
}

bool message_buffer::field_name_equals(unsigned int n, const char* compare_string)
{
  CHECK_CONDITION(n < current_num_fields_, "trying to get name of non-existant field");
  return get_header(n).name_equals(compare_string);
}

bool message_buffer::field_value_equals(unsigned int n, const char* compare_string)
{
  CHECK_CONDITION(n < current_num_fields_, "trying to get value of non-existant field");
  return get_header(n).value_equals(compare_string);
}

size_t message_buffer::get_max_fields()
{
  return max_fields_;
}

bool message_buffer::add_field()
{
  CHECK_CONDITION(current_num_fields_ < get_max_fields(), "cannot add more fields");
  //
  // This is pretty ugly.  The add_field function
  // currently only works with pool allocated headers.
  // assert if using statically allocated headers.
  CHECK_CONDITION(cache_, "using statically-allocated headers where only pool allocated headers are allowed");
  if(cache_->empty()){
    return false;
  }
  cache_->alloc_header(headers_);
  ++current_num_fields_;
  CHECK_CONDITION(headers_.size() == current_num_fields_, "postcondition failed: field counts disagree");
  return true;
}

bool message_buffer::get_header_index_by_name(const char* header_name, unsigned int& index)
{
  //
  // It would be better if this used a map instead of a vector,
  // so it wouldn't be necessary to do a linear search.  The
  // client should cache the result anyway.  Plus since the number
  // of headers shouldn't exceed 50, and it is an in memory
  // operation, it probably won't matter anyway.
  std::list<header_buffer>::iterator current = headers_.begin();
  std::list<header_buffer>::iterator end = headers_.end();
  for(int i = 0; current != end; ++current, ++i){
    if(current->name_equals(header_name)){
      index = i;
      return true;
    }
  }
  return false;
}

size_t message_buffer::get_num_fields()
{
  return current_num_fields_;
}


header_cache* message_buffer::get_header_cache()
{
  return cache_;
}
  
bool message_buffer::get_header_value(const char* header_name, std::string& header_value)
{
  unsigned int index_of_header;

  if(get_header_index_by_name(header_name, index_of_header))
  {
    get_field_value(index_of_header).append_to_string(header_value);
    return true;
  }
  return false;
}

std::list<boost::asio::const_buffer> message_buffer::get_const_buffers()
{
  std::list<boost::asio::const_buffer> buffers;
  boost::asio::const_buffer new_line_buf(new_line, strlen(new_line));
  boost::asio::const_buffer field_sep_buf(field_sep, strlen(field_sep));
  boost::asio::const_buffer space_buf(space, strlen(space));
  
  
  buffers.push_back(start_line1_.get_const_buffer());
  buffers.push_back(space_buf);
  buffers.push_back(start_line2_.get_const_buffer());
  buffers.push_back(space_buf);
  buffers.push_back(start_line3_.get_const_buffer());
  buffers.push_back(new_line_buf);
  headers_.begin();
  std::list<header_buffer>::iterator cur = headers_.begin();
  for(;cur != headers_.end(); ++cur){
    buffers.push_back(cur->get_name().get_const_buffer());
    buffers.push_back(field_sep_buf);
    buffers.push_back(cur->get_value().get_const_buffer());
    buffers.push_back(new_line_buf);
  }
  buffers.push_back(new_line_buf);
  return buffers;
}

} //namespace http
} //namespace switchflow

