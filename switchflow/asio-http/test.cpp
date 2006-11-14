//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <iostream>

/**

ip::tcp::resolver::iterator iter)
#include <boost/asio/detail/pooled_list.hpp>
#include <boost/asio/detail/pooled_hash_map.hpp>
**/

#include <http/header_cache.hpp>
#include <http/message_buffer.hpp>
#include <http/request_buffer_wrapper.hpp>

#include "http_client.hpp"
//
// Make the demuxer global, so I don't have to pass it around.
//boost::asio::demuxer d;


void doit(unsigned long* count)
{
  ++(*count);
  if(!(*count % 100000)){
    std::cout<<*count<<std::endl;
  }
}

// 
// create a long running loop and see what happens.
void postit(unsigned long* count)
{
  ++(*count);
  if(!(*count % 100000)){
    std::cout<<*count<<std::endl;
  }
  if(*count < 100000000){
    
  }
}


template<typename it>
void dump(it begin, it end)
{
  for(;begin != end; ++begin){
    std::cout<<*begin<<" ";
  }
  std::cout<<std::endl;
}

/**
void test_pool()
{
  using namespace boost::asio::detail;
  
  pooled_list<int> test_pooled_list(1000);
  pooled_list<int>::iterator begin = test_pooled_list.begin();
  test_pooled_list.insert(begin, 10);
  test_pooled_list.insert(begin, 13);
  test_pooled_list.insert(begin, 15);

  dump(test_pooled_list.begin(), test_pooled_list.end());
  test_pooled_list.erase(test_pooled_list.begin());
  dump(test_pooled_list.begin(), test_pooled_list.end());
  
  *test_pooled_list.begin() = 42;
  dump(test_pooled_list.begin(), test_pooled_list.end());
  
  test_pooled_list.clear();
  dump(test_pooled_list.begin(), test_pooled_list.end());
  
  const pooled_list<int>& const_pooled_list = test_pooled_list;
//  pooled_list<int>::const_iterator const_begin = const_pooled_list.begin();
//  pooled_list<int>::const_iterator const_end = const_pooled_list.end();

  //dump(const_begin, const_end);

  
  pooled_hash_map<int, int> test_map2; 

  //hash_map<int, int> test_map3;


  test_map2.insert(pooled_hash_map<int, int>::value_type(2, 4));
  test_map2.insert(pooled_hash_map<int, int>::value_type(4, 8));
  test_map2.insert(pooled_hash_map<int, int>::value_type(8, 16));
  
  std::cout<<test_map2.find(2)->second<<std::endl;
  std::cout<<test_map2.find(4)->second<<std::endl;
  std::cout<<test_map2.find(8)->second<<std::endl;
}

int test_queue_overrun()
{
  unsigned long call_count = 0;
  unsigned long post_count = 0;
  
  //
  // post messages until we run out memory and then
  // run them.
  try{
    std::cout<<"running queue limit test...  posting events..."<<std::endl;
    for(post_count = 1;;++post_count){
      //
      // Internally allocates queueing structures, and
      // a copy of the functor returned by bind.
//      d.post(boost::bind(doit, &call_count));
      if(!(post_count % 100000)){
        //
        // print out something to show that
        // we are still alive.
        std::cout<<post_count<<std::endl;
      }
    }
  }
  catch(...){
    //
    // Let's not do anything here in case we throw again.
  }
  //
  // There is a reasonable chance we might throw here,
  // because we are pretty much out of memory.  Might
  // want to comment this line out.
  std::cout<<"caught exception. post_count: "
           <<post_count<<std::endl;
  try{
//    d.run();
  }
  catch(...){

    std::cout<<"caught exception running events."<<std::endl;

    //
    // Can't really do anything graceful here like continue
    // to handle connected sockets.
    //
    // I could try to call run again, but I doubt that's safe.
    // as we could have been mucking with internal structures
    // when the exception let go.
  }

  std::cout<<"all done"<<std::endl;
  std::cout<<"post_count: "<<post_count
           <<std::endl;
  std::cout<<"call_count: "
           <<call_count<<std::endl;

  return 0;
}
**/

class connection_error: public i_connect_error_handler
{
  void timeout(){}
  void connect_failed(){}
  void dns_failed(){}
};

class http_handler: public i_http_connection_handler
{
  void send_failed(){}
  void invalid_peer_header(){}
  void invalid_peer_body(){}
  
  void timeout(){}
  void shutdown(){}
  
  void headers_complete(){}

  void request_complete()
  {
    std::cout<<std::endl<<"---- response complete ----"<<std::endl;
  }

  
  void accept_peer_body(asio::mutable_buffer& buffer)
  {
    std::cout.write(asio::buffer_cast<char*>(buffer), asio::buffer_size(buffer));
  }  
};


void build_request(http::message_buffer& get_request)
{
  http::HTTPRequest_buffer_wrapper request(get_request);
  
  request.get_method().append_from_string("GET"); 
  request.get_uRI().append_from_string("/");
  
  request.get_hTTPVersion_buffer().append_from_string("HTTP/1.1");

  get_request.add_field();
  get_request.get_field_name(0).append_from_string("Host");

  get_request.get_field_value(0).append_from_string("baus.net:8080");
  
  get_request.add_field();
  get_request.get_field_name(1).append_from_string("User-Agent");
  get_request.get_field_value(1).append_from_string("Feed_flow/0.9");
  get_request.add_field();
  get_request.get_field_name(2).append_from_string("Connection");
  get_request.get_field_value(2).append_from_string("close");

  //
  // We now only right feeds to disk if we need
  // the feed data to determine if the feed has been
  // updated.  If we want to keep a history of the
  // feed for other reasons, remove this condition
#define ONLY_WRITE_FEEDS_TO_DETERMINE_UPDATE  
#ifdef ONLY_WRITE_FEEDS_TO_DETERMINE_UPDATE
  bool conditional_get = true;
#else
  bool conditional_get = p_crawl_request_->have_feed_on_disk_;
#endif
  
  int field_num = 3;
  //
  // Only do conditional get if we already have the feed on disk, otherwise the feed
  // will never be retrieved.
  /**
  if(p_crawl_request_->old_last_modified_.size() > 0 && conditional_get){
    get_request.add_field();
    get_request.get_field_name_n(field_num).append_from_string("If-Modified-Since");
    get_request.get_field_value_n(field_num).append_from_string(p_crawl_request_->old_last_modified_.c_str());
    ++field_num;
  }
  if(p_crawl_request_->old_etag_.size() > 0 && conditional_get){
    get_request.add_field();
    get_request.get_field_name_n(field_num).append_from_string("If-None-Match");
    get_request.get_field_value_n(field_num).append_from_string(p_crawl_request_->old_etag_.c_str());
    ++field_num;
  }
  **/
}

int main(int argc, char* argv[])
{

  http::init();
  asio::io_service d;
  connection_error error_handler;
  http_handler handler;
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

  //test_pool();
  
  //return test_queue_overrun();
  
  return 0;
}
