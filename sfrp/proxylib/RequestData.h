//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

/** 
 * -*- Mode: C++ -*-
 * Definition of request_data class
 */
#ifndef REQUEST_DATA_H
#define REQUEST_DATA_H

#include <map>
#include <string>

namespace proxylib{
  
/**
 * request_data encapsulates data that is passed from the request processor
 * to the response processor.  It uses key/value pairs so that the
 * proxylib isn't tied to HTTP layer.  I thought about using a pure virtual
 * class, but it requires that the HTTP layer downcast to the appropriate
 * type, which I thought was ugly.  
 *
 * The other option would be to templatize proxy_handler on this type.  That
 * honestly isn't a bad option, but it requires putting all the logic for the
 * proxy_handler in the header file.  I can't bring myself to do that.  
 */
class request_data
{
public:
  request_data();
  virtual ~request_data();
  set_value();
private:
  std::map<std::string, std::string> key_values_;
};

} // namespace proxylib 

#endif // REQUEST_DATA_H
