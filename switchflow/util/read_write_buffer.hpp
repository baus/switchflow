//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SF_BUFFER_H
#define SF_BUFFER_H

#include <vector>
#include <string>
#include <asio/buffer.hpp>

typedef unsigned char BYTE;
typedef std::vector<BYTE> raw_buffer;

void init_raw_buffer(raw_buffer& buffer, const char* init_value);

class read_write_buffer
{
public:

  read_write_buffer(unsigned int physical_length);

  read_write_buffer(raw_buffer* p_static_buffer);

  read_write_buffer(const read_write_buffer& rhs);

  read_write_buffer();
  
  ~read_write_buffer();

  void set_static_buffer(raw_buffer* p_static_buffer);
  
  raw_buffer& get_raw_buffer();
  
  void reset();

  void set_working_length(unsigned int working_length);

  unsigned int get_working_length();
  
  unsigned int size();
  
  unsigned int get_physical_length();
  
  BYTE& operator[](const unsigned int i);

  void copy_from_buffer(raw_buffer::iterator begin, raw_buffer::iterator end);

  void append_from_buffer(raw_buffer::iterator begin, raw_buffer::iterator end);

  void append_from_string(const char* str);
  
  void append_to_string(std::string& s);
  void append_to_string(std::string& s, unsigned int begin, unsigned int end);
  raw_buffer::iterator begin();

  raw_buffer::iterator working_end();
  bool fully_written();

  void set_process_position(unsigned int processed_position);

  void set_write_position(unsigned int write_position);
  void set_write_end_position(unsigned int write_end_position);
  unsigned int get_write_position();
  unsigned int get_write_end_position();
  unsigned int get_process_position();
  bool fits_in_buffer(unsigned int size);
  bool equals(const char* compare_string);

  char asci_tolower(char c);
  
  enum COMPARE_RESULT{
    SUBSTRING,
    EQUAL,
    NOT_EQUAL
  };
  
  COMPARE_RESULT compare_no_case(const char* compare_string);  

  asio::const_buffer get_const_buffer();
private:
  void operator=(const read_write_buffer& buffer);
  raw_buffer* p_buffer_;
  bool b_using_static_buffer_;
  unsigned int working_length_;
  unsigned int process_position_;
  unsigned int write_end_position_;
  unsigned int write_position_;
};


#endif
