// Class for reading named values from configuration files
// Richard J. Wagner  v2.1  24 May 2004  wagnerr@umich.edu

// Copyright (c) 2004 Richard J. Wagner
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef SF_CONFIGFILE_H
#define SF_CONFIGFILE_H

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "logger.hpp"

class config_file {
public:
	config_file();
	
	bool parse_file(const char* filename);
  void dump(std::ostream& os);
  struct value
  {
  public:

    friend class config_file;
    friend class std::vector<value>;
    friend class std::map<std::string, value>;

    enum value_type{
      INVALID,
      RAW_VALUE,
      ARRAY,
      RECORD
    };
    
    template<class T> 
    T read() const;

    
    const value& operator[](const char* rec_name) const;
    const value& operator[](size_t index) const;
    size_t get_array_size() const;

    
  private:
    value(){}
    
    std::string raw_value_;
    std::vector<value> array_;
    std::map<std::string, value> record_;
    value_type type_;
  };

  const value& operator[](const char* key) const;
private:

  std::istream& read(std::istream& is);

  std::string array_start_;
  std::string array_end_;
  std::string subkey_delimiter_;

  // separator between key and value
  std::string value_delimiter_;
  
  // separator between value and comments
  std::string comment_;      
  
  std::map<std::string,value> contents_;  // extracted keys and values

  typedef std::map<std::string, value> mapt;
  typedef std::map<std::string, value>::iterator mapi;
  typedef std::map<std::string, value>::const_iterator mapci;

  template<class T> static std::string T_as_string( const T& t );
  template<class T> static T string_as_T( const std::string& s );

  void set_delimiters();
  std::string parse_key_name(std::string& key);
  void parse_key(config_file::value& value, std::string& key, const std::string& strvalue);
  value::value_type parse_keytype(std::string& key);
  bool parse_array(std::string& key, size_t& value);
  std::string parse_record_name(std::string& key);
  static void trim( std::string& s );
  void dump_value(std::ostream& os, const std::string& key, const config_file::value& value, int depth);
};


template<class T>
T config_file::value::read() const
{
  CHECK_CONDITION(this->type_ == RAW_VALUE, "Not a raw value");
	return string_as_T<T>( raw_value_ );
}





/* static */
template<class T>
std::string config_file::T_as_string( const T& t )
{
	// Convert from a T to a string
	// Type T must support << operator
	std::ostringstream ost;
	ost << t;
	return ost.str();
}


/* static */
template<class T>
T config_file::string_as_T( const std::string& s )
{
	// Convert from a string to a T
	// Type T must support >> operator
	T t;
	std::istringstream ist(s);
	ist >> t;
	return t;
}


/* static */
template<>
inline std::string config_file::string_as_T<std::string>( const std::string& s )
{
	// Convert from a string to a string
	// In other words, do nothing
	return s;
}


/* static */
template<>
inline bool config_file::string_as_T<bool>( const std::string& s )
{
	// Convert from a string to a bool
	// Interpret "false", "F", "no", "n", "0" as false
	// Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true
	bool b = true;
	std::string sup = s;
	for( std::string::iterator p = sup.begin(); p != sup.end(); ++p )
		*p = toupper(*p);  // make string all caps
	if( sup==std::string("FALSE") || sup==std::string("F") ||
		sup==std::string("NO") || sup==std::string("N") ||
		sup==std::string("0") || sup==std::string("NONE") )
		b = false;
	return b;
}





#endif  // CONFIGFILE_H
