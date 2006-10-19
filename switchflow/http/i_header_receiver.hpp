#ifndef HTTPMessageReceiver_H__
#define HTTPMessageReceiver_H__

#include <util/read_write_buffer.hpp>

#include "http.hpp"

namespace http{

class IHeaderReceiver
{
 public:
  virtual STATUS startLineToken1(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
  virtual STATUS startLineToken2(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
  virtual STATUS startLineToken3(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete) = 0;
  virtual STATUS setFieldName(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
  virtual STATUS setFieldValue(read_write_buffer& buffer, int iBegin, int iEnd, bool bComplete)=0;
  virtual STATUS endFields() = 0;
  
};

} // namespace httplib
#endif
