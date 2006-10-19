#ifndef __HTTPMessageReceiver_H__
#define __HTTPMessageReceiver_H__

#include "Buffer.h"

class IHTTPMessageReceiver
{
  public:
    virtual bool startLineToken1(Buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual bool startLineToken2(Buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual bool startLineToken3(Buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual bool setFieldName(Buffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual bool setFieldValue(Buffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual bool endFields() = 0;
    virtual bool setBody(Buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual bool setChunked() = 0;
};

class IRequestReceiver{
  public:
    virtual void setVersion(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setMethod(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setURI(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setFieldName(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual void setFieldValue(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual void setBody(RawBuffer& buffer, int iBegin, int iEnd, bool moreData) = 0;
};

class IResponseReceiver{
  public:
    virtual void setVersion(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setReturnCode(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setStatus(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
    virtual void setFieldName(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual void setFieldValue(RawBuffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
    virtual void setBody(RawBuffer& buffer, int iBegin, int iEnd, bool moreData) = 0;
};

#endif
