#include <iostream>

#include <http/token_parser.hpp>
#include <http/line_parser.hpp>

using namespace switchflow;
using namespace switchflow::http;

std::string line;

receive::status token_receiver(asio::const_buffer buffer, char delimiter, buffer_status buf_status)
{
    std::cout<<"In token receiver"<<std::endl;
    return receive::SUCCESS;
}

receive::status line_receiver(asio::const_buffer buffer, buffer_status buf_status)
{
    for(int i = 0; i < asio::buffer_size(buffer); ++i)
    {
        line += asio::buffer_cast<const char*>(buffer)[i];
    }
    if(buf_status == COMPLETE)
    {
        std::cout<<line<<std::endl;
        line = "";
    }
    return receive::SUCCESS;
}

std::set<char> s_space_delimiters;
std::set<char> s_cr_delimiters;

int main(int argc, char* argv[])
{
  s_space_delimiters.insert(32);
  s_cr_delimiters.insert(13);

  const char* t1= "GE";
  const char* t2= "T / ";
  const char* t3= "HTTP/1.1\r";
  const char* t4="\n test 2\r\n test 3\r\n ";
  
  asio::const_buffer buf1(t1, 2);
  asio::const_buffer buf2(t2, 4);
  asio::const_buffer buf3(t3, 9);
  asio::const_buffer buf4(t4, strlen(t4));

  line_parser parser(50, s_cr_delimiters);
  parse_result result;
  do{
  result = parser.parse(buf1, line_receiver);
  buf1 = result.buffer;
  }while(asio::buffer_size(result.buffer) > 0);
 
  do{
  result = parser.parse(buf2, line_receiver);
  buf2 = result.buffer;
  }while(asio::buffer_size(result.buffer) > 0);

  do{
  result = parser.parse(buf3, line_receiver);
  buf3 = result.buffer;
  }while(asio::buffer_size(result.buffer) > 0);
  
  do{
  result = parser.parse(buf4, line_receiver);
  buf4 = result.buffer;
  }while(asio::buffer_size(result.buffer) > 0);

  return 0;
}
