//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// 
// Copyright (C) Christopher Baus 2004-2005.  All rights reserved.
#ifndef PROXYSTREAMINTERFACE_H
#define PROXYSTREAMINTERFACE_H

// library includes
#include <socketlib/connection.hpp>
#include <socketlib/status.hpp>

class pipeline_data_queue;

namespace proxylib{
  class ProxyHandler;
  
class ProxyStreamInterface{  
 public:

  //
  // Check the state of the destination.
  //
  // returns true only if the socket hasn't been connected,
  // and is not in the process of connecting.  
  bool isDestDisconnected();
    
  
  //
  // This fowards the buffer to the destination. If WOULD_BLOCK
  // is returned then control should return back to poll loop.  
  // Failing to return to the poll loop could cause the server to
  // lock.
  socketlib::STATUS forward(read_write_buffer& buf);

  //
  // Force any buffered data to be flushed to the network.
  void flush(); 
 
  const char* getSrcAddress();

  const char* getDestAddress();

  socketlib::connection* getDest();

  pipeline_data_queue* get_pipeline_data_queue();
  
  //
  // Construct a ProxyStreamInterface.  It is
  // private since it should only be constructed by
  // ProxyHandler.  
  ProxyStreamInterface();

  //
  // This resets the private members of the class. This
  // would typically be done on construction, but since the
  // objects get recycled, and need to be reset, it is done through
  // this private member function.  
  //
  // @param pProxyHandler backpointer to the ProxyHandler
  // @param pSrcData data for the stream socket source 
  // @param pDestData data for the stream socket destination 
  void reset(ProxyHandler* pProxyHandler, socketlib::connection* pSrcData, socketlib::connection* pDestData);

 private:
  // ProxyHandler must be a friend, so it can call the following functions.
  friend class ProxyHandler;
    
    
   
    
  //
  // back pointer to the proxy handler
  ProxyHandler* m_pProxyHandler;

  socketlib::connection* m_pSrcData;
    
  socketlib::connection* m_pDestData;
};

} // namespace proxylib

#endif // PROXYSTREAMINTERFACE_H
