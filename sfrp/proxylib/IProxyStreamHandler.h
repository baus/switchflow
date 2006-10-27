//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus 2004-2005  All rights reserved.
#ifndef IPROXYSTREAMHANDLER_H
#define IPROXYSTREAMHANDLER_H

#include <assert.h>
#include <netinet/in.h>

namespace proxylib{
/**
 * This interface allows application layer clients to receive 
 * messages from the proxy stream.  All application layers 
 * required to implement this interface.  
 *
 * There are two instances of ProxyStreamHandler per instance of ProxyHandler --
 * one to handle data being transfered from the client to the
 * server (request stream), and one to handle data being transfered 
 * from the server to the client (response stream). 
 *
 * It is possible to have different handlers for the request stream and response
 * stream.  For instance it might be useful to validate requests, but to 
 * simply forward responses.
 */
class IProxyStreamHandler
{
 public:
    IProxyStreamHandler():m_pProxyStreamInterface(0){}

    enum FORWARD_ADDRESS_STATUS
    {
      // This stream doesn't provide forward address.  This
      // is used by response streams.
      STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS,
      
      // Enough information hasn't been received to determine
      // the forward address
      FORWARD_ADDRESS_NOT_YET_AVAILABLE,
      
      // This connection doesn't provide a forward address.
      // This is returned by HTTP/1.0 streams which don't
      // have host headers.
      CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS,

      // Unable to retrieve the forward address.  Usually
      // means the client has provided a host that we don't
      // know about.
      ERROR_DETERMINING_FORWARD_ADDRESS,

      // The forward address is available, and it is
      // possible to connect to the server.
      FORWARD_ADDRESS_AVAILABLE
    };
    
    //
    // These next two functions are related.  They provide support for
    // just in time connection to the downstream server, based
    // on data already read in the stream.  For instance, in
    // HTTP this data would be in the header information.
    virtual FORWARD_ADDRESS_STATUS getForwardAddressStatus() = 0;


    //
    // Returns the forward address as determined by data in the
    // stream.
    //
    // This should only be called if forwardAddressAvailableFromStream
    // returns true.  
    virtual sockaddr& getForwardAddressFromStream() = 0;
    
    virtual socketlib::STATUS processData(read_write_buffer& buf) = 0;
    virtual IProxyStreamHandler* clone() = 0;
    ProxyStreamInterface* GetProxyStreamInterface(){assert(m_pProxyStreamInterface);return m_pProxyStreamInterface;}
    virtual void reset() = 0;
    virtual void reset(ProxyStreamInterface* pProxyStreamInterface )
    {
      m_pProxyStreamInterface = pProxyStreamInterface;
      reset();
    }
    
 private:
    ProxyStreamInterface* m_pProxyStreamInterface;
};

} // proxylib

#endif // IPROXYSTREAMHANDLER_H
