#include <iostream>

#include <http/uri_query_parser.hpp>
#include <http/i_uri_query_receiver.hpp>
#include <util/logger.hpp>
#include <string>
#include <vector>

using namespace switchflow;
using namespace switchflow::http;

class test_handler: public i_uri_query_receiver
{
public:
  virtual STATUS key(asio::const_buffer buffer, bool complete)
  {
    std::cout<<"key buffer size: "<<asio::buffer_size(buffer)<<std::endl;
    std::cout<<"key complete: "<<(complete?"true":"false")<<std::endl;
    dump_buffer(buffer);
      
  }

  virtual STATUS value(asio::const_buffer buffer, bool complete)
  {
    std::cout<<"value buffer size: "<<asio::buffer_size(buffer)<<std::endl;
    std::cout<<"value complete: "<<(complete?"true":"false")<<std::endl;
    dump_buffer(buffer);

  }

  void dump_buffer(asio::const_buffer buffer)
  {
    const char* raw_buffer = asio::buffer_cast<const char*>(buffer);
    std::string token;
    for(int i = 0 ; i < asio::buffer_size(buffer); ++i)
      {
        token += raw_buffer[i];
      }
    std::cout<<token<<std::endl;
   
  }
};

int main(int argc, char* argv[])
{
  http::init();

  const char* segments[5] = {"fo", "o=%20b", "ar&", "foobar2", "=baz"};

  std::vector<asio::const_buffer> buffers;
  
  test_handler my_handler;
  uri_query_parser parser(my_handler, 256, 256);
  
  for(int i = 0; i < 5; ++i)
  {
    parser.parse_uri_query(asio::const_buffer(segments[i], strlen(segments[i])));
  }
  
  return 0;
}

// should output:
//
//key buffer size: 2
//key complete: false
//fo
//key buffer size: 1
//key buffer complete: true
//o
//value buffer size: 4
//value buffer complete: false
//%20b
//value buffer size: 2
//value buffer complete: true
//ar
//key buffer size: 7
//key buffer complete: false
//foobar2
//key buffer size: 0
//key buffer complete: true
//
//value buffer size: 3
//value buffer complete: true
//baz
