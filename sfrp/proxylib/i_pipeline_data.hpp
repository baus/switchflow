//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

///
///-*- Mode: C++ -*-
/// Definition of i_pipeline_data class

#ifndef IPIPELINEDATA_HPP
#define IPIPELINEDATA_HPP

namespace proxylib{

///
/// Defines the interface used by the proxy_handler to pass data
/// between the request and response pipelines. This was
/// found to be necessary generate a Combined Log Entry log entry
/// because information was required from the request and the response.
///
/// Right now there is only a virtual destructor, so the proxylib
/// can delete entries if the connection is dropped.
///
/// Eventually the entries should be repooled.   
///  
class i_pipeline_data
{
 public:
  virtual void reset(){};
  virtual ~i_pipeline_data(){}
};

} // proxylib

#endif // IPIPELINEDATA_HPP
