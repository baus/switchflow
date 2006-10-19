#include "NonValidatingProxyStreamHandler.h"

socketlib::STATUS NonValidatingProxyStreamHandler::processData(read_write_buffer& buf)
{
  return GetProxyStreamInterface()->forward(buf);
}

NonValidatingProxyStreamHandler::NonValidatingProxyStreamHandler():IProxyStreamHandler()
{
}

NonValidatingProxyStreamHandler* NonValidatingProxyStreamHandler::clone()
{
  return 0;//NonValidatingProxyStreamHandler;
}
