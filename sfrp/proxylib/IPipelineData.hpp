//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

///
///-*- Mode: C++ -*-
/// Definition of IPipelineData class
///
/// christopher <christopher@baus.net>
///
/// Copyright (C) Summit Sage Designs, LLC.  All rights reserved.
///
#ifndef IPIPELINEDATA_HPP
#define IPIPELINEDATA_HPP

namespace proxylib{

///
/// Defines the interface used by the ProxyHandler to pass data
/// between the request and response pipelines. This was
/// found to be necessary generate a Combined Log Entry log entry
/// because information was required from the request and the response.
///
/// Right now there is only a virtual destructor, so the proxylib
/// can delete entries if the connection is dropped.
///
/// Eventually the entires should be repooled.   
///  
class IPipelineData
{
 public:
  virtual ~IPipelineData(){}
};

} // proxylib

#endif // IPIPELINEDATA_HPP
