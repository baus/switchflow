//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// i_data provider allows the retrieval of data
// to be extensible.  The client decides how
// data will be retrieved and presented to the parser.
//
//
#ifndef __I_DATA_PROVIDER_H__
#define __I_DATA_PROVIDER_H__
class i_data_provider 
{
  public:
    //
    // return a vector of length. 
    //
    // The parser expects this function to block.
    // While it might be better to implement a state 
    // machine in the future that allows the parser to
    // implemented in a non-blocking fashion, using multiple
    // blocking threads is far easier right now.  I suspect
    // that eventually I will have to make this non-blocking
    // and use probably 3 threads in the entire system, but 
    // that is unlikely to happen any time soon.
    //
    // Keep in mind that, using select, etc., it might be
    // possible to implement a super fast HTTP proxy.  That 
    // may be the only way to reach wire speed for, say,
    // a major backbone provider.
    // 
    //
    virtual int get_data(raw_buffer& buffer) = 0;

};
#endif // __I_DATA_PROVIDER_H__
