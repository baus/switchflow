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

read_write_buffer::read_write_buffer(unsigned int physicalLength):bUsingStaticBuffer_(false)
{
  pBuffer_ = new raw_buffer(physicalLength);
  reset();
}

read_write_buffer::~read_write_buffer()
{
  if(!bUsingStaticBuffer_){
    delete pBuffer_;
  }
}

read_write_buffer::read_write_buffer(raw_buffer* pStaticBuffer):bUsingStaticBuffer_(true)
{
  pBuffer_ = pStaticBuffer;
  reset();
}

read_write_buffer::read_write_buffer():bUsingStaticBuffer_(true)
{
  pBuffer_ = NULL;
  reset();
}

read_write_buffer::read_write_buffer(const read_write_buffer& rhs)
{
  bUsingStaticBuffer_ = rhs.bUsingStaticBuffer_;
  if(bUsingStaticBuffer_){
    pBuffer_ = rhs.pBuffer_;
    if(pBuffer_){
      workingLength_ = pBuffer_->size();
    }
    else{
      workingLength_ = 0;
    }
  }
  else{
    pBuffer_ = new raw_buffer(rhs.pBuffer_->size());
  }
  reset();
}


void read_write_buffer::set_static_buffer(raw_buffer* pStaticBuffer)
{
  if(!bUsingStaticBuffer_){
    delete pBuffer_;
  }
  pBuffer_ = pStaticBuffer;
  bUsingStaticBuffer_ = true;
  reset();
}

raw_buffer::iterator read_write_buffer::begin()
{
  return pBuffer_->begin();
}

raw_buffer& read_write_buffer::get_raw_buffer()
{
  return *pBuffer_;
}

void read_write_buffer::set_working_length(unsigned int workingLength)
{
  CHECK_CONDITION(workingLength <= pBuffer_->size(), "buffer consistency");
  CHECK_CONDITION(!bUsingStaticBuffer_, "buffer type");
  workingLength_ = workingLength;
  processPosition_ = 0;
  //
  // write end position has to be overridden manually.
  //
  setWriteEndPosition(workingLength);
  setWritePosition(0);
}

void read_write_buffer::setWriteEndPosition(unsigned int writeEndPosition)
{
  CHECK_CONDITION(writeEndPosition <= workingLength_, "buffer consistency");
  writeEndPosition_ = writeEndPosition;
}

unsigned int read_write_buffer::getWriteEndPosition()
{
  return writeEndPosition_;
}

unsigned int read_write_buffer::getProcessPosition()
{
  return processPosition_;
}

void read_write_buffer::setProcessPosition(unsigned int processedPosition)
{
  CHECK_CONDITION(processedPosition <= workingLength_, "buffer consistency");
  processPosition_ = processedPosition;
}

bool read_write_buffer::fullyWritten()
{
  return writePosition_ >= workingLength_;
}

unsigned int read_write_buffer::getWorkingLength()
{
  return workingLength_;
}
 
unsigned int read_write_buffer::size()
{
  return workingLength_;
}

raw_buffer::iterator read_write_buffer::workingEnd()
{
  return begin() + workingLength_;
}

BYTE& read_write_buffer::operator[](const unsigned int i)
{
  CHECK_CONDITION(i >= 0 && i < workingLength_, "buffer consistency");
  return (*pBuffer_)[i];
}



void read_write_buffer::copyFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!bUsingStaticBuffer_, "buffer type");
  CHECK_CONDITION(static_cast<int>(end - begin) <= static_cast<int>(pBuffer_->size()), "buffer consistency");
  workingLength_ = end - begin;
  memcpy(&((*pBuffer_)[0]), &*begin, workingLength_);
}

void read_write_buffer::appendFromBuffer(raw_buffer::iterator begin, raw_buffer::iterator end)
{
  CHECK_CONDITION(!bUsingStaticBuffer_, "buffer type");
  CHECK_CONDITION(static_cast<int>(workingLength_ + end - begin) <= static_cast<int>(pBuffer_->size()), "buffer consistency");
  memcpy(&((*pBuffer_)[workingLength_]), &*begin, end - begin);
  workingLength_ += (end - begin);
  writeEndPosition_ = workingLength_;
}

unsigned int read_write_buffer::getPhysicalLength()
{
  return pBuffer_->size();
}


bool read_write_buffer::fitsInBuffer(unsigned int size)
{
  return ((getWorkingLength() + size) <= getPhysicalLength());
}

void read_write_buffer::reset()
{
  if(!bUsingStaticBuffer_){
    workingLength_ = 0;
    writeEndPosition_ = 0;
  }
  else if(pBuffer_){
    writeEndPosition_ = pBuffer_->size();
    workingLength_ = pBuffer_->size();
  }
  else{
    writeEndPosition_ = 0;
    workingLength_ = 0;
  }
  processPosition_ = 0;
  writePosition_ = 0;
}

void read_write_buffer::setWritePosition(unsigned int writePosition)
{
  CHECK_CONDITION(writePosition <= workingLength_, "buffer consistency");
  writePosition_ = writePosition;
}

unsigned int read_write_buffer::getWritePosition()
{
  return writePosition_;
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
    (*pBuffer_)[i] = *str;
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
  CHECK_CONDITION(begin <= end && end <= workingLength_, "buffer consistency");
  unsigned int num_chars = end - begin;
  s.reserve(s.size() + num_chars);
  for(int i = begin; i < end; ++i){
    s += (*pBuffer_)[i];
  } 
}

asio::const_buffer read_write_buffer::get_const_buffer()
{
  return asio::const_buffer(&((*pBuffer_)[writePosition_]), writeEndPosition_);
}
