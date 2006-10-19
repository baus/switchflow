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
  m_pProxyHandler(0),
  m_pSrcData(0), 
  m_pDestData(0)
{
}

bool ProxyStreamInterface::isDestDisconnected()
{
  assert(m_pDestData == NULL);
  return m_pDestData->state() == socketlib::connection::NOT_CONNECTED;
}

socketlib::STATUS ProxyStreamInterface::forward(read_write_buffer& buf)
{
  return m_pDestData->non_blocking_write(buf);
}
    
void ProxyStreamInterface::reset(ProxyHandler* pProxyHandler, 
                                 socketlib::connection* pSrcData,
                                 socketlib::connection* pDestData)
{
//  assert(pProxyHandler != 0);
//  assert(pSrcData != 0);
//  assert(pDestData != 0);
  
  m_pSrcData = pSrcData;
  m_pDestData = pDestData;
  m_pProxyHandler = pProxyHandler;
}

const char* ProxyStreamInterface::getSrcAddress()
{
  return m_pSrcData->get_ip_addr();
}

const char* ProxyStreamInterface::getDestAddress()
{
  return m_pDestData->get_ip_addr();
}

void ProxyStreamInterface::flush()
{
  return m_pDestData->flush();
}

pipeline_data_queue* ProxyStreamInterface::get_pipeline_data_queue()
{
  return &m_pProxyHandler->pipeline_data_queue_;
}


socketlib::connection* ProxyStreamInterface::getDest()
{
  return m_pDestData;
}

} // namespace proxylib
