//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (c) Christopher Baus.  All Rights Reserved
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <locale>

#include "logger.hpp"
#include "read_write_buffer.hpp"

typedef std::vector<BYTE> raw_buffer;

read_write_buffer::read_write_buffer(unsigned int physicalLength):m_bUsingStaticBuffer(false)
{
  m_pBuffer = new raw_buffer(physicalLength);
  reset();
}

read_write_buffer::~read_write_buffer()
{
  if(!m_bUsingStaticBuffer){
    delete m_pBuffer;
  }
}

read_write_buffer::read_write_buffer(raw_buffer* pStaticBuffer):m_bUsingStaticBuffer(true)
{
  m_pBuffer = pStaticBuffer;
  reset();
}

read_write_buffer::read_write_buffer():m_bUsingStaticBuffer(true)
{
  m_pBuffer = NULL;
  reset();
}

read_write_buffer::read_write_buffer(const read_write_buffer& rhs)
{
  m_bUsingStaticBuffer = rhs.m_bUsingStaticBuffer;
  if(m_bUsingStaticBuffer){
    m_pBuffer = rhs.m_pBuffer;
    if(m_pBuffer){
      m_workingLength = m_pBuffer->size();
    }
    else{
      m_workingLength = 0;
    }
  }
  else{
    m_pBuffer = new raw_buffer(rhs.m_pBuffer->size());
  }
  reset();
}


void read_write_buffer::set_static_buffer(raw_buffer* pStaticBuffer)
{
  if(!m_bUsingStaticBuffer){
    delete m_pBuffer;
  }
  m_pBuffer = pStaticBuffer;
  m_bUsingStaticBuffer = true;
  reset();
}

raw_buffer::iterator read_write_buffer::begin()
{
  return m_pBuffer->begin();
}

raw_buffer& read_write_buffer::get_raw_buffer()
{
  return *m_pBuffer;
}

void read_write_buffer::set_working_length(unsigned int workingLength)
{
  CHECK_CONDITION(workingLength <= m_pBuffer->size(), "buffer consistency");
  CHECK_CONDITION(!m_bUsingStaticBuffer, "buffer type");
  m_workingLength = workingLength;
  m_processPosition = 0;
  //
  // write end position has to be overridden manually.
  //
  setWriteEndPosition(workingLength);
  setWritePosition(0);
}

void read_write_buffer::setWriteEndPosition(unsigned int writeEndPosition)
{
  CHECK_CONDITION(writeEndPosition <= m_workingLength, "buffer consistency");
  m_writeEndPosition = writeEndPosition;
}

unsigned int read_write_buffer::getWriteEndPosition()
{
  return m_writeEndPosition;
}

unsigned int read_write_buffer::getProcessPosition()
{
  return m_processPosition;
}

void read_write_buffer::setProcessPosition(unsigned int processedPosition)
{
  CHECK_CONDITION(processedPosition <= m_workingLength, "buffer consistency");
  m_processPosition = processedPosition;
}

bool read_write_buffer::fullyWritten()
{
  return m_writePosition >= m_workingLength;
}

unsigned int read_write_buffer::getWorkingLength()
{
  return m_workingLength;
}
 
unsigned int read_write_buffer::size()
{
  return m_workingLength;
}

raw_buffer::iterator read_write_buffer::workingEnd()
{
  return begin() + m_workingLength;
}

BYTE& read_write_buffer::operator[](const unsigned int i)
{
  CHECK_CONDITION(i >= 0 && i < m_workingLength, "buffer consistency");
  return (*m_pBuffer)[i];
}



void read_write_buffer::copyFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!m_bUsingStaticBuffer, "buffer type");
  CHECK_CONDITION(static_cast<int>(end - begin) <= static_cast<int>(m_pBuffer->size()), "buffer consistency");
  m_workingLength = end - begin;
  memcpy(&((*m_pBuffer)[0]), &*begin, m_workingLength);
}

void read_write_buffer::appendFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!m_bUsingStaticBuffer, "buffer type");
  CHECK_CONDITION(static_cast<int>(m_workingLength + end - begin) <= static_cast<int>(m_pBuffer->size()), "buffer consistency");
  memcpy(&((*m_pBuffer)[m_workingLength]), &*begin, end - begin);
  m_workingLength += (end - begin);
  m_writeEndPosition = m_workingLength;
}

unsigned int read_write_buffer::getPhysicalLength()
{
  return m_pBuffer->size();
}


bool read_write_buffer::fitsInBuffer(unsigned int size)
{
  return ((getWorkingLength() + size) <= getPhysicalLength());
}

void read_write_buffer::reset()
{
  if(!m_bUsingStaticBuffer){
    m_workingLength = 0;
    m_writeEndPosition = 0;
  }
  else if(m_pBuffer){
    m_writeEndPosition = m_pBuffer->size();
    m_workingLength = m_pBuffer->size();
  }
  else{
    m_writeEndPosition = 0;
    m_workingLength = 0;
  }
  m_processPosition = 0;
  m_writePosition = 0;
}

void read_write_buffer::setWritePosition(unsigned int writePosition)
{
  CHECK_CONDITION(writePosition <= m_workingLength, "buffer consistency");
  m_writePosition = writePosition;
}

unsigned int read_write_buffer::getWritePosition()
{
  return m_writePosition;
}

void read_write_buffer::appendToString(std::string& s)
{
  appendToString(s, 0, getWorkingLength());
}

bool read_write_buffer::equals(const char* compareString)
{
  return compareNoCase(compareString) == EQUAL;
}

char read_write_buffer::asci_tolower(char c)
{
  if(c >= 'A' && c <= 'Z')
  {
    return 'a' + c - 'A'; 
  }
  return c;
}

read_write_buffer::COMPARE_RESULT read_write_buffer::compareNoCase(const char* compareString)
{
  int workingLength = getWorkingLength();
  int i;
  char currentChar;
  for(i = 0, currentChar = *compareString;
      i < workingLength && currentChar != 0;
      ++i, currentChar = *(compareString + i))
  {
    if(asci_tolower((*this)[i]) != asci_tolower(currentChar)){
      return NOT_EQUAL;
    }
  }
  if(i == workingLength && currentChar == 0){
    return EQUAL;
  }
  return SUBSTRING;
}

void read_write_buffer::appendFromString(const char * str)
{
  size_t len = strlen(str);
  CHECK_CONDITION(len <= getPhysicalLength(), "buffer consistency");
  int i;
  for(i = 0; i < len; ++i,++str){
    (*m_pBuffer)[i] = *str;
  }
  set_working_length(len);
  setWriteEndPosition(len);
}

void init_raw_buffer(raw_buffer& buffer, const char* init_value)
{
  CHECK_CONDITION(strlen(init_value) <= buffer.size(), "validate buffer length");
  memcpy(&buffer[0], init_value, buffer.size());
}

void read_write_buffer::appendToString(std::string& s, unsigned int begin, unsigned int end)
{
  CHECK_CONDITION(begin <= end && end <= m_workingLength, "buffer consistency");
  unsigned int num_chars = end - begin;
  s.reserve(s.size() + num_chars);
  for(int i = begin; i < end; ++i){
    s += (*m_pBuffer)[i];
  } 
}

asio::const_buffer read_write_buffer::get_const_buffer()
{
  return asio::const_buffer(&((*m_pBuffer)[m_writePosition]), m_writeEndPosition);
}
