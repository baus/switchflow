//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef __I_HTTP_MESSAGE_RECEIVER_H__
#define __I_HTTP_MESSAGE_RECEIVER_H__

#include "buffer.h"

class i_http_message_receiver
{
  public:
    virtual bool start_line_token1(buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual bool start_line_token2(buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual bool start_line_token3(buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual bool set_field_name(buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual bool set_field_value(buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual bool end_fields() = 0;
    virtual bool set_body(buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual bool set_chunked() = 0;
};

class i_request_receiver{
  public:
    virtual void set_version(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_method(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_uri(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_field_name(raw_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual void set_field_value(raw_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual void set_body(raw_buffer& buffer, int i_begin, int i_end, bool more_data) = 0;
};

class i_response_receiver{
  public:
    virtual void set_version(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_return_code(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_status(raw_buffer& buffer, int i_begin, int i_end, bool b_complete) = 0;
    virtual void set_field_name(raw_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual void set_field_value(raw_buffer& buffer, int i_begin, int i_end, bool b_complete)=0;
    virtual void set_body(raw_buffer& buffer, int i_begin, int i_end, bool more_data) = 0;
};

#endif // __I_HTTP_MESSAGE_RECEIVER_H__
