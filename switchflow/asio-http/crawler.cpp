//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <Python.h>

#include <iostream>


#include <http/header_cache.hpp>
#include <http/message_buffer.hpp>
#include <http/request_buffer_wrapper.hpp>


#include "http_client.hpp"
#include "crawler.hpp"


class connection_error: public i_connect_error_handler
{
  void timeout(){}
  void connect_failed(){}
  void dns_failed(){}
};

python_handler_module::python_handler_module()
{
  PyObject *p_name;
    
  Py_InitializeEx(0);

  p_name = PyString_FromString("crawler");
  p_module_ = PyImport_Import(p_name);
  Py_DECREF(p_name);
  if(p_module_ == NULL){
    PyErr_Print();
    std::cerr<<"Failed to load python module: crawler"<<std::endl;
  }
  if(p_module_ != NULL)
  {
  p_func_ = PyObject_GetAttrString(p_module_, "handle_response");
  if(!p_func_ || !PyCallable_Check(p_func_)){
    std::cerr<<"Failed to create python callback function"<<std::endl;
    Py_XDECREF(p_func_);
        Py_DECREF(p_module_);
  }
  }
}

python_handler_module::~python_handler_module()
{
  Py_XDECREF(p_func_);
    
  // What if the module failed to load?
  Py_DECREF(p_module_);
  Py_Finalize();

}
int python_handler_module::execute_response_callback(const char* response)
{
  PyObject * p_args;
  PyObject * p_value;
  int return_val = 0;

  assert(p_func_ || PyCallable_Check(p_func_));
  p_args = PyTuple_New(1);
  p_value = PyString_FromString(response);
  PyTuple_SetItem(p_args, 0, p_value);
  p_value = PyObject_CallObject(p_func_, p_args);
  Py_DECREF(p_args);
  if (p_value != NULL) {
    return_val = PyInt_AsLong(p_value);
    Py_DECREF(p_value);
  }
  else {
    //
    // error occured...
  }
  return return_val;
}



void crawler_handler::request_complete()
{
  std::cout<<std::endl<<"---- response complete ----"<<std::endl;
  python_handler_module_.execute_response_callback(feed_.c_str());
}

  
void crawler_handler::accept_peer_body(asio::mutable_buffer& buffer)
{
    feed_.insert(feed_.length(), asio::buffer_cast<char*>(buffer), asio::buffer_size(buffer));
  //std::cout.write(asio::buffer_cast<char*>(buffer), asio::buffer_size(buffer));
}  


void build_request(http::message_buffer& get_request)
{
  http::HTTPRequestBufferWrapper request(get_request);
  
  request.getMethod().appendFromString("GET"); 
  request.getURI().appendFromString("/");
  
  request.getHTTPVersionBuffer().appendFromString("HTTP/1.1");

  get_request.add_field();
  get_request.get_field_name(0).appendFromString("Host");

  get_request.get_field_value(0).appendFromString("feeds.baus.net");
  
  get_request.add_field();
  get_request.get_field_name(1).appendFromString("User-Agent");
  get_request.get_field_value(1).appendFromString("FeedFlow/1.1.1");
  get_request.add_field();
  get_request.get_field_name(2).appendFromString("Connection");
  get_request.get_field_value(2).appendFromString("close");

  //
  // We now only right feeds to disk if we need
  // the feed data to determine if the feed has been
  // updated.  If we want to keep a history of the
  // feed for other reasons, remove this condition
#define ONLY_WRITE_FEEDS_TO_DETERMINE_UPDATE  
#ifdef ONLY_WRITE_FEEDS_TO_DETERMINE_UPDATE
  bool conditionalGET = true;
#else
  bool conditionalGET = p_crawl_request_->have_feed_on_disk_;
#endif
  
  int field_num = 3;
  //
  // Only do conditional get if we already have the feed on disk, other wise the feed
  // will never be retrieved.
  /**
  if(p_crawl_request_->old_last_modified_.size() > 0 && conditionalGET){
    get_request.add_field();
    get_request.getFieldNameN(field_num).appendFromString("If-Modified-Since");
    get_request.getFieldValueN(field_num).appendFromString(p_crawl_request_->old_last_modified_.c_str());
    ++field_num;
  }
  if(p_crawl_request_->old_etag_.size() > 0 && conditionalGET){
    get_request.add_field();
    get_request.getFieldNameN(field_num).appendFromString("If-None-Match");
    get_request.getFieldValueN(field_num).appendFromString(p_crawl_request_->old_etag_.c_str());
    ++field_num;
  }
  **/
}

int main(int argc, char* argv[])
{
  python_handler_module python_module;

  http::init();
  asio::io_service d;
  connection_error error_handler;
  crawler_handler handler(python_module);
  http::header_cache cache(5000, 256, 256);
  http_client client(d,
                     "baus.net", 
                     80,
                     cache,
                     error_handler,
                     handler);


  
  http::message_buffer request(&cache, 3, 256, 10, 50);

  build_request(request);

  client.make_request(request);

  d.run();
  python_module.execute_response_callback("test 123");

  return 0;
 
}

