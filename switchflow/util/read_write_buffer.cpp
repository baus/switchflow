//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <locale>

#include "logger.hpp"
#include "read_write_buffer.hpp"

typedef std::vector<BYTE> raw_buffer;

read_write_buffer::read_write_buffer(unsigned int physical_length):b_using_static_buffer_(false)
{
  p_buffer_ = new raw_buffer(physical_length);
  reset();
}

read_write_buffer::~read_write_buffer()
{
  if(!b_using_static_buffer_){
    delete p_buffer_;
  }
}

read_write_buffer::read_write_buffer(raw_buffer* p_static_buffer):b_using_static_buffer_(true)
{
  p_buffer_ = p_static_buffer;
  reset();
}

read_write_buffer::read_write_buffer():b_using_static_buffer_(true)
{
  p_buffer_ = NULL;
  reset();
}

read_write_buffer::read_write_buffer(const read_write_buffer& rhs)
{
  b_using_static_buffer_ = rhs.b_using_static_buffer_;
  if(b_using_static_buffer_){
    p_buffer_ = rhs.p_buffer_;
    if(p_buffer_){
      working_length_ = p_buffer_->size();
    }
    else{
      working_length_ = 0;
    }
  }
  else{
    p_buffer_ = new raw_buffer(rhs.p_buffer_->size());
  }
  reset();
}


void read_write_buffer::set_static_buffer(raw_buffer* p_static_buffer)
{
  if(!b_using_static_buffer_){
    delete p_buffer_;
  }
  p_buffer_ = p_static_buffer;
  b_using_static_buffer_ = true;
  reset();
}

raw_buffer::iterator read_write_buffer::begin()
{
  return p_buffer_->begin();
}

raw_buffer& read_write_buffer::get_raw_buffer()
{
  return *p_buffer_;
}

void read_write_buffer::set_working_length(unsigned int working_length)
{
  CHECK_CONDITION(working_length <= p_buffer_->size(), "buffer consistency");
  CHECK_CONDITION(!b_using_static_buffer_, "buffer type");
  working_length_ = working_length;
  process_position_ = 0;
  //
  // write end position has to be overridden manually.
  //
  set_write_end_position(working_length);
  set_write_position(0);
}

void read_write_buffer::set_write_end_position(unsigned int write_end_position)
{
  CHECK_CONDITION(write_end_position <= working_length_, "buffer consistency");
  write_end_position_ = write_end_position;
}

unsigned int read_write_buffer::get_write_end_position()
{
  return write_end_position_;
}

unsigned int read_write_buffer::get_process_position()
{
  return process_position_;
}

void read_write_buffer::set_process_position(unsigned int processed_position)
{
  CHECK_CONDITION(processed_position <= working_length_, "buffer consistency");
  process_position_ = processed_position;
}

bool read_write_buffer::fully_written()
{
  return write_position_ >= working_length_;
}

unsigned int read_write_buffer::get_working_length()
{
  return working_length_;
}
 
unsigned int read_write_buffer::size()
{
  return working_length_;
}

raw_buffer::iterator read_write_buffer::working_end()
{
  return begin() + working_length_;
}

BYTE& read_write_buffer::operator[](const unsigned int i)
{
  CHECK_CONDITION(i >= 0 && i < working_length_, "buffer consistency");
  return (*p_buffer_)[i];
}



void read_write_buffer::copy_from_buffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!b_using_static_buffer_, "buffer type");
  CHECK_CONDITION(static_cast<int>(end - begin) <= static_cast<int>(p_buffer_->size()), "buffer consistency");
  working_length_ = end - begin;
  memcpy(&((*p_buffer_)[0]), &*begin, working_length_);
}

void read_write_buffer::append_from_buffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!b_using_static_buffer_, "buffer type");
  CHECK_CONDITION(static_cast<int>(working_length_ + end - begin) <= static_cast<int>(p_buffer_->size()), "buffer consistency");
  memcpy(&((*p_buffer_)[working_length_]), &*begin, end - begin);
  working_length_ += (end - begin);
  write_end_position_ = working_length_;
}

unsigned int read_write_buffer::get_physical_length()
{
  return p_buffer_->size();
}


bool read_write_buffer::fits_in_buffer(unsigned int size)
{
  return ((get_working_length() + size) <= get_physical_length());
}

void read_write_buffer::reset()
{
  if(!b_using_static_buffer_){
    working_length_ = 0;
    write_end_position_ = 0;
  }
  else if(p_buffer_){
    write_end_position_ = p_buffer_->size();
    working_length_ = p_buffer_->size();
  }
  else{
    write_end_position_ = 0;
    working_length_ = 0;
  }
  process_position_ = 0;
  write_position_ = 0;
}

void read_write_buffer::set_write_position(unsigned int write_position)
{
  CHECK_CONDITION(write_position <= working_length_, "buffer consistency");
  write_position_ = write_position;
}

unsigned int read_write_buffer::get_write_position()
{
  return write_position_;
}

void read_write_buffer::append_to_string(std::string& s)
{
  append_to_string(s, 0, get_working_length());
}

bool read_write_buffer::equals(const char* compare_string)
{
  return compare_no_case(compare_string) == EQUAL;
}

char read_write_buffer::asci_tolower(char c)
{
  if(c >= 'A' && c <= 'Z')
  {
    return 'a' + c - 'A'; 
  }
  return c;
}

read_write_buffer::COMPARE_RESULT read_write_buffer::compare_no_case(const char* compare_string)
{
  int working_length = get_working_length();
  int i;
  char current_char;
  for(i = 0, current_char = *compare_string;
      i < working_length && current_char != 0;
      ++i, current_char = *(compare_string + i))
  {
    if(asci_tolower((*this)[i]) != asci_tolower(current_char)){
      return NOT_EQUAL;
    }
  }
  if(i == working_length && current_char == 0){
    return EQUAL;
  }
  return SUBSTRING;
}

void read_write_buffer::append_from_string(const char * str)
{
  size_t len = strlen(str);
  CHECK_CONDITION(len <= get_physical_length(), "buffer consistency");
  int i;
  for(i = 0; i < len; ++i,++str){
    (*p_buffer_)[i] = *str;
  }
  set_working_length(len);
  set_write_end_position(len);
}

void init_raw_buffer(raw_buffer& buffer, const char* init_value)
{
  CHECK_CONDITION(strlen(init_value) <= buffer.size(), "validate buffer length");
  memcpy(&buffer[0], init_value, buffer.size());
}

void read_write_buffer::append_to_string(std::string& s, unsigned int begin, unsigned int end)
{
  CHECK_CONDITION(begin <= end && end <= working_length_, "buffer consistency");
  unsigned int num_chars = end - begin;
  s.reserve(s.size() + num_chars);
  for(int i = begin; i < end; ++i){
    s += (*p_buffer_)[i];
  } 
}

asio::const_buffer read_write_buffer::get_const_buffer()
{
  return asio::const_buffer(&((*p_buffer_)[write_position_]), write_end_position_);
}
