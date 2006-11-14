//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef __TEST_HTTP_REQUEST_HANDLERS_H__
#define __TEST_HTTP_REQUEST_HANDLERS_H__


class test_http_server_request_handlers: public i_message_body_receiver, public i_field_receiver, public i_request_header_receiver, public i_data_provider
{
  public:
    Test_hTTPServer_request_handlers(ACE_SOCK_Stream& stream):stream_(stream){}

  
  virtual void set_field_name(buffer& buffer, int i_begin, int i_end, bool b_complete)
  {
  }
  
  virtual void set_field_value(buffer& buffer, int i_begin, int i_end, bool b_complete)
  {
  }

  virtual void set_version(buffer& buffer, int i_begin, int i_end, bool b_complete)
  {
  }

  virtual void set_method(buffer& buffer, int i_begin, int i_end, bool b_complete)
  {
  }

  virtual void set_uri(buffer& buffer, int i_begin, int i_end, bool b_complete)
  {
  }

  virtual void set_body(buffer& buffer, int i_begin, int i_end, bool more_data)
  {
  }

  virtual int get_data(raw_buffer& buffer)
  {
    buffer.resize(1500);
    return stream_.recv(&buffer[0], 1500);
  }

  virtual void set_chunked()
  {
  }

  
  virtual void set_version(const char* version)
  {
    cout << "version:" << version << endl;
  }
  
  virtual void set_method(const char* method)
  {
    cout << "method:" << method << endl;
  }

  virtual void set_uri(const char* uri)
  {
    cout << "URI:" << uri << endl;
  }

  virtual void set_body(raw_buffer& buffer, bool more_data)
  {
  }

    
  virtual void set_field(const char* name, const char* value)
  {
    cout << "field: " << name << endl;
    cout << "value: " << value << endl;
  }

  virtual void end_field()
  {
    cout << "endfield"<<endl;
    cout<<"HTTP 1.0 200 Success"<<endl;
    //
    // I think this is a valid response
    //
    stream_.send_n("HTTP 1.0 200 Success", 20);
    stream_.send_n(&inet_util::CR, 1);
    stream_.send_n(&inet_util::LF, 1);
    stream_.send_n(&inet_util::CR, 1);
    stream_.send_n(&inet_util::LF, 1);
    stream_.send_n("return from Switch_flow Reverse Proxy HTTP server", 41);
  }

  
  private:
    ACE_SOCK_Stream stream_;

};



#endif // __TEST_HTTP_REQUEST_HANDLERS_H__
