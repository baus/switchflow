/**
 * InetUtil
 *
 * © Copyright 2003 Summit Sage, LLC
 * All Rights Reserved                  
 *
 * @author Christopher Baus <christopher@summitsage.com>
 */
#ifndef __TESTHTTPREQUESTHANDLERS_H__
#define __TESTHTTPREQUESTHANDLERS_H__


class TestHTTPServerRequestHandlers: public IMessageBodyReceiver, public IFieldReceiver, public IRequestHeaderReceiver, public IDataProvider
{
  public:
    TestHTTPServerRequestHandlers(ACE_SOCK_Stream& stream):m_stream(stream){}

  
  virtual void setFieldName(Buffer& buffer, int iBegin, int iEnd, bool bComplete)
  {
  }
  
  virtual void setFieldValue(Buffer& buffer, int iBegin, int iEnd, bool bComplete)
  {
  }

  virtual void setVersion(Buffer& buffer, int iBegin, int iEnd, bool bComplete)
  {
  }

  virtual void setMethod(Buffer& buffer, int iBegin, int iEnd, bool bComplete)
  {
  }

  virtual void setURI(Buffer& buffer, int iBegin, int iEnd, bool bComplete)
  {
  }

  virtual void setBody(Buffer& buffer, int iBegin, int iEnd, bool moreData)
  {
  }

  virtual int getData(RawBuffer& buffer)
  {
    buffer.resize(1500);
    return m_stream.recv(&buffer[0], 1500);
  }

  virtual void setChunked()
  {
  }

  
  virtual void setVersion(const char* version)
  {
    cout << "version:" << version << endl;
  }
  
  virtual void setMethod(const char* method)
  {
    cout << "method:" << method << endl;
  }

  virtual void setURI(const char* URI)
  {
    cout << "URI:" << URI << endl;
  }

  virtual void setBody(RawBuffer& buffer, bool moreData)
  {
  }

    
  virtual void setField(const char* name, const char* value)
  {
    cout << "field: " << name << endl;
    cout << "value: " << value << endl;
  }

  virtual void endField()
  {
    cout << "endfield"<<endl;
    cout<<"HTTP 1.0 200 Success"<<endl;
    //
    // I think this is a valid response
    //
    m_stream.send_n("HTTP 1.0 200 Success", 20);
    m_stream.send_n(&InetUtil::CR, 1);
    m_stream.send_n(&InetUtil::LF, 1);
    m_stream.send_n(&InetUtil::CR, 1);
    m_stream.send_n(&InetUtil::LF, 1);
    m_stream.send_n("return from Summit Sage Ultra HTTP server", 41);
  }

  
  private:
    ACE_SOCK_Stream m_stream;

};



#endif



