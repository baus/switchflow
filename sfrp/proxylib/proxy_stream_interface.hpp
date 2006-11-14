//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef PROXY_STREAM_INTERFACE_H
#define PROXY_STREAM_INTERFACE_H

// library includes
#include <socketlib/connection.hpp>
#include <socketlib/status.hpp>

class pipeline_data_queue;

namespace proxylib{
  class proxy_handler;
  
class proxy_stream_interface{  
 public:

  //
  // Check the state of the destination.
  //
  // returns true only if the socket hasn't been connected,
  // and is not in the process of connecting.  
  bool is_dest_disconnected();
    
  
  //
  // This fowards the buffer to the destination. If WOULD_BLOCK
  // is returned then control should return back to poll loop.  
  // Failing to return to the poll loop could cause the server to
  // lock.
  socketlib::STATUS forward(read_write_buffer& buf);

  //
  // Force any buffered data to be flushed to the network.
  void flush(); 
 
  const char* get_src_address();

  const char* get_dest_address();

  socketlib::connection* get_dest();

  pipeline_data_queue* get_pipeline_data_queue();
  
  //
  // Construct a proxy_stream_interface.  It is
  // private since it should only be constructed by
  // proxy_handler.  
  proxy_stream_interface();

  //
  // This resets the private members of the class. This
  // would typically be done on construction, but since the
  // objects get recycled, and need to be reset, it is done through
  // this private member function.  
  //
  // @param p_proxy_handler backpointer to the proxy_handler
  // @param p_src_data data for the stream socket source 
  // @param p_dest_data data for the stream socket destination 
  void reset(proxy_handler* p_proxy_handler, socketlib::connection* p_src_data, socketlib::connection* p_dest_data);

 private:
  // proxy_handler must be a friend, so it can call the following functions.
  friend class proxy_handler;
    
    
   
    
  //
  // back pointer to the proxy handler
  proxy_handler* p_proxy_handler_;

  socketlib::connection* p_src_data_;
    
  socketlib::connection* p_dest_data_;
};

} // namespace proxylib

#endif // PROXY_STREAM_INTERFACE_H
