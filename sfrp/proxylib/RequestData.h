/** 
 * -*- Mode: C++ -*-
 * Definition of RequestData class
 *
 * christopher baus <christopher@baus.net>
 *
 * Copyright (C) Summit Sage, LLC.  All rights reserved.
 */
#ifndef REQUEST_DATA_H
#define REQUEST_DATA_H

#include <map>
#include <string>

namespace proxylib{
  
/**
 * RequestData encapsulates data that is passed from the Request processor
 * to the Response processor.  It uses key/value pairs so that the
 * proxylib isn't tied to HTTP layer.  I thought about using a pure virtual
 * class, but it requires that the HTTP layer down casts to the appropriate
 * type, which I thought was ugly.  
 *
 * The other option would be to templatize ProxyHandler on this type.  That
 * honestly isn't a bad option, but it requires putting all the logic for the
 * ProxyHandler in the header file.  I can't bring myself to do that.  
 */
class RequestData
{
public:
  RequestData();
  virtual ~RequestData();
  SetValue();
private:
  std::map<std::string, std::string> m_keyValues;
};

} // namespace proxylib 

#endif // REQUEST_DATA_H
