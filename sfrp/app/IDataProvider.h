//
// IData provider allows the retrieval of data
// to be extensible.  The client decides how
// data will be retrieved and presented to the parser.
//
//
#ifndef __IDataProvider_H__
#define __IDataProvider_H__
class IDataProvider 
{
  public:
    //
    // return a vector of length. 
    //
    // The reason the vector of length 
    //
    // The parser expects this function to block.
    // While it might be better to implement a state 
    // machine in the future that allows the parser to
    // implemented in a non-blocking fashion, using multiple
    // blocking threads is far easier right now.  I suspect
    // that eventually I will have to move this non-blocking
    // with probably 3 threads in the entire system, but 
    // that is unlikely to happen anytime soon.
    //
    // Keep that in mind.  Using select, etc. it might be
    // possible to implement a super fast HTTP proxy.  It is
    // probably the only chance of making it wire speed 
    // say for a major backbone provider.
    // 
    //
    virtual int getData(RawBuffer& buffer) = 0;

};
#endif
