//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// config_file.cpp

#include <string>
#include <fstream>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "logger.hpp"
#include "config_file.hpp"

void config_file::set_delimiters()
{
  array_start_ = "[";
  array_end_ = "]";
  subkey_delimiter_ = ".";
  value_delimiter_ = "=";
  comment_ = "#";
}

bool config_file::parse_file( const char* filename )  
{
  // Construct a config_file, getting keys and values from given file
  std::ifstream in( filename );
  
  if( !in ){
    return false;
  }
  read(in);
  return true;
}


void config_file::dump(std::ostream& os)
{
  mapci cur = contents_.begin();
  for(;cur!=contents_.end();++cur)
  {
    std::string key = cur->first;
    dump_value(os, key, cur->second, 0);
  }
  
}


void config_file::dump_value(std::ostream& os, const std::string& key, const config_file::value& value, int depth)
{
  if(value.type_ == config_file::value::RAW_VALUE)
  {
    os<<key<<"="<<value.raw_value_<<std::endl;
  }
  else if(value.type_ == config_file::value::ARRAY)
  {
    for(int i = 0; i < value.array_.size(); ++i)
    {
      std::string temp(key);
      temp += array_start_;
      temp += boost::lexical_cast<std::string>(i);
      temp += array_end_;
      dump_value(os, temp, value.array_[i], ++depth);
    }
  }
  else if(value.type_ == config_file::value::RECORD)
  {
    mapci cur = value.record_.begin();
    for(;cur!=value.record_.end();++cur)
    {
      
      dump_value(os, key + "." + cur->first, cur->second, ++depth);
    }
  }
}

config_file::config_file()
{
  set_delimiters();
}




/* static */
void config_file::trim( std::string& s )
{
  // Remove leading and trailing whitespace
  static const char whitespace[] = " \n\t\v\r\f";
  s.erase( 0, s.find_first_not_of(whitespace) );
  s.erase( s.find_last_not_of(whitespace) + 1U );
}



std::istream& config_file::read(std::istream& is)
{
  // Load a config_file from is
  // Read in keys and values, keeping internal whitespace
  typedef std::string::size_type pos;
  const std::string& delim  = value_delimiter_;  
  const std::string& comm   = comment_;    // comment
  const pos skip = delim.length();        // length of separator
  
  std::string nextline = "";  // might need to read ahead to see where value ends
  
  while( is || nextline.length() > 0 )
  {
    // Read an entire line at a time
    std::string line;
    if( nextline.length() > 0 )
    {
      line = nextline;  // we read ahead; use it now
      nextline = "";
    }
    else
    {
      std::getline( is, line );
    }
    
    // Ignore comments
    line = line.substr( 0, line.find(comm) );
    
    
    // Parse the line if it contains a delimiter
    pos delim_pos = line.find( delim );
    if( delim_pos < std::string::npos )
    {
      // Extract the key
      std::string key = line.substr( 0, delim_pos );
      line.replace( 0, delim_pos+skip, "" );
      
      // See if value continues on the next line
      // Stop at blank line, next line with a key, end of stream,
      // or end of file sentry
      bool terminate = false;
      while( !terminate && is )
      {
        std::getline( is, nextline );
        terminate = true;
        
        std::string nlcopy = nextline;
        config_file::trim(nlcopy);
        if( nlcopy == "" ) continue;
        
        nextline = nextline.substr( 0, nextline.find(comm) );
        if( nextline.find(delim) != std::string::npos )
          continue;
        
        nlcopy = nextline;
        config_file::trim(nlcopy);
        if( nlcopy != "" ) line += "\n";
        line += nextline;
        terminate = false;
      }
      
      config_file::trim(key);
      config_file::trim(line);
      
      std::string rootkey = parse_key_name(key);
      mapi it = contents_.find(rootkey);
      if(it == contents_.end())
      {
        value new_value;
        parse_key(new_value, key, line);
        contents_[rootkey] = new_value;
      }
      else{
        parse_key(it->second, key, line);
        it = contents_.find(rootkey);
        it->second;
      }
    }
  }
  
  return is;
}

void config_file::parse_key(config_file::value& value, std::string& key, const std::string& strvalue)
{
  value::value_type keytype = config_file::parse_keytype(key);
  value.type_ = keytype;
  
  if(keytype == value::RAW_VALUE){
    value.raw_value_ = strvalue;
  }
  else if(keytype == value::ARRAY){
    size_t index;
    bool ret_val = parse_array(key, index);
    if(index>=value.array_.size())
    {
      value.array_.resize(index + 1);
    }
    parse_key(value.array_[index], key, strvalue);
  }
  else if(keytype == value::RECORD){
    std::string recordname = parse_record_name(key);
    mapi it = value.record_.find(recordname);
    if(it == value.record_.end()){
      config_file::value new_value;
      parse_key(new_value, key, strvalue);
      value.record_.insert(mapt::value_type(recordname, new_value));
    }
    else{
      parse_key(it->second, key, strvalue);
    }
  }
}

std::string config_file::parse_record_name(std::string& key)
{
  key = key.substr(1, key.length());
  return parse_key_name(key);
}


std::string config_file::parse_key_name(std::string& key)
{
  std::string ret_val;
  // Parse the line if it contains a delimiter
  std::string delim = array_start_ + subkey_delimiter_;
  size_t delim_pos = key.find_first_of( delim );
  ret_val = key.substr(0, delim_pos);
  if( delim_pos == std::string::npos )
  {
    delim_pos = key.size();
  }
  
  key = key.substr(delim_pos);
  return ret_val;
}


config_file::value::value_type config_file::parse_keytype(std::string& key)
{
  if(key[0] == array_start_[0])
  {
    return value::ARRAY;
  }
      
  else if(key[0] == subkey_delimiter_[0])
  {
    return value::RECORD;
  }
  return value::RAW_VALUE;
}

bool config_file::parse_array(std::string& key, size_t& value)
{
  if(key[0] != array_start_[0])
  {
    return false;
  }
  key = key.substr(1);
  size_t pos = key.find(array_end_);
  if(pos == std::string::npos)
  {
    return false;
  }

  bool ret_val = true;
  std::string index = key.substr(0, pos);
  key = key.substr(pos + 1);
  try{
    value = boost::lexical_cast<size_t>(index);
  }
  catch(boost::bad_lexical_cast&)
  {
    ret_val = false;
  }
  
  return ret_val;
}

const config_file::value& config_file::value::operator[](size_t index) const
{

  CHECK_CONDITION(this->type_ == ARRAY, "Not an array type");
  CHECK_CONDITION(index < this->array_.size() && index>=0, "Array out of range");
  return this->array_[index];
}

const config_file::value& config_file::value::operator[](const char* rec_name) const
{
  CHECK_CONDITION(this->type_ == RECORD, "Not a record type");
  mapci it = record_.find(rec_name);
  CHECK_CONDITION(it != this->record_.end(), "Record not found");
  return it->second;
}

const config_file::value& config_file::operator[](const char* key) const
{
  mapci it = contents_.find(key);
  CHECK_CONDITION(it != contents_.end(), "Key doesn't exist in configuration file");
  return it->second;
}

size_t config_file::value::get_array_size() const
{
  return array_.size();
}
