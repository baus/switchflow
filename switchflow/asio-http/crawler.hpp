#include "http_client.hpp"

class python_handler_module
{
public:
	python_handler_module();
	~python_handler_module();
	int execute_response_callback(const char * response);
private:
	PyObject* p_module_;
	PyObject* p_func_;
};



class crawler_handler: public i_http_connection_handler
{
public:
	crawler_handler(python_handler_module& python_module):python_handler_module_(python_module){}
  void send_failed(){}
  void invalid_peer_header(){}
  void invalid_peer_body(){}
  
  void timeout(){}
  void shutdown(){}
  
  void headers_complete(){}

  void request_complete();
  
  void accept_peer_body(asio::mutable_buffer& buffer);

private:
  std::string feed_;
  python_handler_module& python_handler_module_;
};


