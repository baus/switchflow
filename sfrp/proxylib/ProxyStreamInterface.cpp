//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
#include <assert.h>

#include <string>

#include <util/logger.hpp>

#include "ProxyHandler.h"
#include "ProxyStreamInterface.h"


//
// @class ProxyStreamInterface proxystreaminterface.h
// @author <a href="mailto:christopher@baus.net">christopher</a>
namespace proxylib{

ProxyStreamInterface::ProxyStreamInterface():
  pProxyHandler_(0),
  pSrcData_(0), 
  pDestData_(0)
{
}

bool ProxyStreamInterface::isDestDisconnected()
{
  assert(pDestData_ == NULL);
  return pDestData_->state() == socketlib::connection::NOT_CONNECTED;
}

socketlib::STATUS ProxyStreamInterface::forward(read_write_buffer& buf)
{
  return pDestData_->non_blocking_write(buf);
}
    
void ProxyStreamInterface::reset(ProxyHandler* pProxyHandler, 
                                 socketlib::connection* pSrcData,
                                 socketlib::connection* pDestData)
{
//  assert(pProxyHandler != 0);
//  assert(pSrcData != 0);
//  assert(pDestData != 0);
  
  pSrcData_ = pSrcData;
  pDestData_ = pDestData;
  pProxyHandler_ = pProxyHandler;
}

const char* ProxyStreamInterface::getSrcAddress()
{
  return pSrcData_->get_ip_addr();
}

const char* ProxyStreamInterface::getDestAddress()
{
  return pDestData_->get_ip_addr();
}

void ProxyStreamInterface::flush()
{
  return pDestData_->flush();
}

pipeline_data_queue* ProxyStreamInterface::get_pipeline_data_queue()
{
  return &pProxyHandler_->pipeline_data_queue_;
}


socketlib::connection* ProxyStreamInterface::getDest()
{
  return pDestData_;
}

} // namespace proxylib
