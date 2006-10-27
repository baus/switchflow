//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved
#include <util/logger.hpp>
#include <string>

#include "message_buffer.hpp"

typedef std::vector<BYTE> RawBuffer;


namespace http{
  
  const char* message_buffer::new_line = "\r\n";
  const char* message_buffer::field_sep = ": ";
  const char* message_buffer::space = " ";

  message_buffer::message_buffer(header_cache* cache,
                                 unsigned int maxStartLine1Length, 
                                 unsigned int maxStartLine2Length, 
                                 unsigned int maxStartLine3Length,
                                 unsigned int maxNumHeaders):
    startLine1_(maxStartLine1Length),
    startLine2_(maxStartLine2Length), 
    startLine3_(maxStartLine3Length), 
    max_fields_(maxNumHeaders),
    current_num_fields_(0),
    cache_(cache)
{
}
  

  message_buffer::message_buffer(RawBuffer* startLine1,
                                 RawBuffer* startLine2,
                                 RawBuffer* startLine3,
                                 std::list<header_buffer*>& fields):
    startLine1_(startLine1),
    startLine2_(startLine2),
    startLine3_(startLine3),
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
  startLine1_.reset();
  startLine2_.reset();
  startLine3_.reset();
}

bool message_buffer::append_to_status_line_1(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  if(startLine1_.fitsInBuffer(end - begin)){
    startLine1_.appendFromBuffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_status_line_2(RawBuffer::iterator begin, RawBuffer::iterator end)
{
  if(startLine2_.fitsInBuffer(end - begin)){
    startLine2_.appendFromBuffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_status_line_3(RawBuffer::iterator begin, RawBuffer::iterator end)
{

  if(startLine3_.fitsInBuffer(end - begin)){
    startLine3_.appendFromBuffer(begin, end);
    return true;
  }
  return false;
}

bool message_buffer::append_to_name(unsigned int n, RawBuffer::iterator begin, RawBuffer::iterator end)
{
  header_buffer& header = get_header(n);
  if(header.getName().fitsInBuffer(end - begin)){
    header.append_to_name(begin, end);
    return true;
  }
  return false;
}

header_buffer& message_buffer::get_header(int n)
{
  assert(n < current_num_fields_);
  
  std::list<header_buffer>::iterator current = headers_.begin();

  for(int i = 0; i < n; ++current, ++i);
  
  return *current;
}

bool message_buffer::append_to_value(unsigned int n, RawBuffer::iterator begin, RawBuffer::iterator end)
{
  
  header_buffer& header = get_header(n);
  if(header.getValue().fitsInBuffer(end - begin)){
    header.append_to_value(begin, end);
    return true;
  }
  return false;
}


read_write_buffer& message_buffer::get_status_line_1()
{
  return startLine1_;
}

read_write_buffer& message_buffer::get_status_line_2()
{
  return startLine2_;
}

read_write_buffer& message_buffer::get_status_line_3()
{
  return startLine3_;
}


read_write_buffer& message_buffer::get_field_name(unsigned int n)
{
  assert(n < current_num_fields_);
  return get_header(n).getName();
}

read_write_buffer& message_buffer::get_field_value(unsigned int n)
{
  assert(n < current_num_fields_);
  return get_header(n).getValue();
}

bool message_buffer::field_name_equals(unsigned int n, const char* compareString)
{
  assert(n < current_num_fields_);
  return get_header(n).name_equals(compareString);
}

bool message_buffer::field_value_equals(unsigned int n, const char* compareString)
{
  assert(n < current_num_fields_);
  return get_header(n).value_equals(compareString);
}

size_t message_buffer::get_max_fields()
{
  return max_fields_;
}

bool message_buffer::add_field()
{
  assert(current_num_fields_ < get_max_fields());
  //
  // This is pretty ugly.  The add_field function
  // currently only works with pool allocated headers.
  // assert if using statically allocated headers.
  assert(cache_);
  if(cache_->empty()){
    return false;
  }
  cache_->alloc_header(headers_);
  ++current_num_fields_;
  assert(headers_.size() == current_num_fields_);
  return true;
}

bool message_buffer::get_header_index_by_name(const char* headerName, unsigned int& index)
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
    if(current->name_equals(headerName)){
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
    get_field_value(index_of_header).appendToString(header_value);
    return true;
  }
  return false;
}

std::list<asio::const_buffer> message_buffer::get_const_buffers()
{
  std::list<asio::const_buffer> buffers;
  asio::const_buffer new_line_buf(new_line, strlen(new_line));
  asio::const_buffer field_sep_buf(field_sep, strlen(field_sep));
  asio::const_buffer space_buf(space, strlen(space));
  
  
  buffers.push_back(startLine1_.get_const_buffer());
  buffers.push_back(space_buf);
  buffers.push_back(startLine2_.get_const_buffer());
  buffers.push_back(space_buf);
  buffers.push_back(startLine3_.get_const_buffer());
  buffers.push_back(new_line_buf);
  headers_.begin();
  std::list<header_buffer>::iterator cur = headers_.begin();
  for(;cur != headers_.end(); ++cur){
    buffers.push_back(cur->getName().get_const_buffer());
    buffers.push_back(field_sep_buf);
    buffers.push_back(cur->getValue().get_const_buffer());
    buffers.push_back(new_line_buf);
  }
  buffers.push_back(new_line_buf);
  return buffers;
}

}

