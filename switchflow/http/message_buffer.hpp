//
// Copyright (c) Christopher Baus. All Rights Reserved
#ifndef SSVS_MESSAGEBUFFER_H
#define SSVS_MESSAGEBUFFER_H

#include <list>
#include <string>
#include <map>

#include <util/read_write_buffer.hpp>
#include "header_buffer.hpp"
#include "header_cache.hpp"

typedef unsigned char BYTE;
typedef std::vector<BYTE> RawBuffer;

namespace http{
  
class message_buffer
{
public:
  //
  // This constructor creates a message_buffer
  // in which the headers are dynamically allocated
  // via the cache.  
  message_buffer(header_cache* cache,
                 unsigned int maxStartLine1Length, 
                 unsigned int maxStartLine2Length, 
                 unsigned int maxStartLine3Length,
                 unsigned int maxNumHeaders);

  
  
  message_buffer(RawBuffer* startLine1,
                 RawBuffer* startLine2,
                 RawBuffer* startLine3,
                 std::list<header_buffer*>& fields);

  ~message_buffer();
  
  void reset();
  bool append_to_status_line_1(RawBuffer::iterator begin, RawBuffer::iterator end);
  bool append_to_status_line_2(RawBuffer::iterator begin, RawBuffer::iterator end);
  bool append_to_status_line_3(RawBuffer::iterator begin, RawBuffer::iterator end);

  bool append_to_name(unsigned int n, RawBuffer::iterator begin, RawBuffer::iterator end);
  bool append_to_value(unsigned int n, RawBuffer::iterator begin, RawBuffer::iterator end);
   
  read_write_buffer& get_status_line_1();
  read_write_buffer& get_status_line_2();
  read_write_buffer& get_status_line_3();
  bool add_field();
  size_t get_num_fields();
  size_t get_max_fields();
  read_write_buffer& get_field_name(unsigned int n);
  read_write_buffer& get_field_value(unsigned int n);
  
  bool field_name_equals(unsigned int n, const char* compareString);
  bool field_value_equals(unsigned int n, const char* compareString);

  bool get_header_index_by_name(const char* headerName, unsigned int& index);

  header_cache* get_header_cache();

  bool get_header_value(const char* header_name, std::string& header_value);
  
  std::list<asio::const_buffer> get_const_buffers();
 
  
  const static char* new_line;
  const static char* field_sep;
  const static char* space;
private:
  header_buffer& get_header(int n);
  bool fitsInBuffer(read_write_buffer& buf, int size);
  unsigned int current_num_fields_;
  unsigned int max_fields_;
  read_write_buffer m_startLine1;
  read_write_buffer m_startLine2;
  read_write_buffer m_startLine3;
  std::list<header_buffer> m_headers;
  header_cache* cache_;
};

}
#endif
