//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef IPROXYSTREAMHANDLER_H
#define IPROXYSTREAMHANDLER_H

#include <assert.h>
#include <netinet/in.h>

namespace proxylib{
/**
 * This interface allows application layer clients to receive 
 * messages from the proxy stream.  All application layers are
 * required to implement this interface.  
 *
 * There are two instances of proxy_stream_handler per instance of proxy_handler --
 * one to handle data being transfered from the client to the
 * server (request stream), and one to handle data being transfered 
 * from the server to the client (response stream). 
 *
 * It is possible to have different handlers for the request stream and response
 * stream.  For instance it might be useful to validate requests, but to 
 * simply forward responses.
 */
class i_proxy_stream_handler
{
 public:
    i_proxy_stream_handler():p_proxy_stream_interface_(0){}

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
    virtual FORWARD_ADDRESS_STATUS get_forward_address_status() = 0;


    //
    // Returns the forward address as determined by data in the
    // stream.
    //
    // This should only be called if forward_address_available_from_stream
    // returns true.  
    virtual sockaddr& get_forward_address_from_stream() = 0;
    
    virtual socketlib::STATUS process_data(read_write_buffer& buf) = 0;
    virtual i_proxy_stream_handler* clone() = 0;
    proxy_stream_interface* Get_proxy_stream_interface(){assert(p_proxy_stream_interface_);return p_proxy_stream_interface_;}
    virtual void reset() = 0;
    virtual void reset(proxy_stream_interface* p_proxy_stream_interface )
    {
      p_proxy_stream_interface_ = p_proxy_stream_interface;
      reset();
    }
    
 private:
    proxy_stream_interface* p_proxy_stream_interface_;
};

} // proxylib

#endif // IPROXYSTREAMHANDLER_H
