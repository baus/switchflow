#include <iostream>

#include <http/line_parser.hpp>
#include <http/i_line_receiver.hpp>
#include <util/logger.hpp>
#include <string>
#include <vector>

using namespace switchflow;
using namespace switchflow::http;

class test_handler: public i_line_receiver
{
public:

  STATUS dump_result(asio::const_buffer buffer, bool b_complete)
    {
      std::cout<<"buffer size: "<<asio::buffer_size(buffer)<<std::endl;
      
      const char* raw_buffer = asio::buffer_cast<const char*>(buffer);
      std::string token;
      for(int i = 0 ; i < asio::buffer_size(buffer); ++i)
      {
        token += raw_buffer[i];
      }
      std::cout<<token<<std::endl;
      
      return COMPLETE;
    
    }
  virtual STATUS receive_line(asio::const_buffer buffer, bool b_complete)
    {      return dump_result(buffer, b_complete); }
};

int main(int argc, char* argv[])
{
  http::init();

  const char* t1= "GE";
  const char* t2= "T / ";
  const char* t3= "HTTP/1.1\r";
  const char* t4="\n";
  
  asio::const_buffer buf1(t1, 2);
  asio::const_buffer buf2(t2, 4);
  asio::const_buffer buf3(t3, 9);
  asio::const_buffer buf4(t4, 1);

  test_handler my_handler;
  line_parser parser(my_handler, 30);
  CHECK_CONDITION(parser.parse_line(buf1).status == INCOMPLETE, "wrong return value 1");
  CHECK_CONDITION(parser.parse_line(buf2).status == INCOMPLETE, "wrong return value 2");
  CHECK_CONDITION(parser.parse_line(buf3).status == INCOMPLETE, "wrong return value 3");
  CHECK_CONDITION(parser.parse_line(buf4).status == COMPLETE, "wrong return value 4");

  return 0;
}
