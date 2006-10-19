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

  unsigned int getWorkingLength();
  
  unsigned int size();
  
  unsigned int getPhysicalLength();
  
  BYTE& operator[](const unsigned int i);

  void copyFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end);

  void appendFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end);

  void appendFromString(const char* str);
  
  void appendToString(std::string& s);
  void appendToString(std::string& s, unsigned int begin, unsigned int end);
  raw_buffer::iterator begin();

  raw_buffer::iterator workingEnd();
  bool fullyWritten();

  void setProcessPosition(unsigned int processed_position);

  void setWritePosition(unsigned int write_position);
  void setWriteEndPosition(unsigned int write_end_position);
  unsigned int getWritePosition();
  unsigned int getWriteEndPosition();
  unsigned int getProcessPosition();
  bool fitsInBuffer(unsigned int size);
  bool equals(const char* compare_string);

  char asci_tolower(char c);
  
  enum COMPARE_RESULT{
    SUBSTRING,
    EQUAL,
    NOT_EQUAL
  };
  
  COMPARE_RESULT compareNoCase(const char* compare_string);  

  asio::const_buffer get_const_buffer();
private:
  void operator=(const read_write_buffer& buffer);
  raw_buffer* m_pBuffer;
  bool m_bUsingStaticBuffer;
  unsigned int m_workingLength;
  unsigned int m_processPosition;
  unsigned int m_writeEndPosition;
  unsigned int m_writePosition;
};


#endif
