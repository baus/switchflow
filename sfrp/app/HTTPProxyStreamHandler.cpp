//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <stdlib.h>
#include <netdb.h>
#include <boost/cast.hpp>

#include <util/logger.hpp>
#include <http/static_strings.hpp>
#include <http/request_buffer_wrapper.hpp>
#include <http/response_buffer_wrapper.hpp>

#include "HTTPProxyStreamHandler.h"
#include "CombinedLogRecord.hpp"
#include "pipeline_data.hpp"
#include <string>

HTTPProxyStreamHandler::HTTPProxyStreamHandler(http::header_cache* cache,
                                               const std::map<std::string, std::pair<httplib::URL, bool> >& host_map,
                                               http::STREAM_TYPE streamType,
                                               unsigned int maxStartLine1Length, 
                                               unsigned int maxStartLine2Length, 
                                               unsigned int maxStartLine3Length,
                                               unsigned int numHeaders,
                                               unsigned int maxHeaderNameLength,
                                               unsigned int maxHeaderValueLength,
                                               AccessLog* pAccessLog,                                               
                                               i_request_postprocessor* p_postprocessor):
  cache_(cache),
  host_map_(host_map),
  m_streamType(streamType),
  m_headerParser(&header_handler_),
  m_bodyParser(this),
  m_currentField(0),
  m_currentDumpHeader(0),
  m_maxStartLine1Length(maxStartLine1Length),
  m_maxStartLine2Length(maxStartLine2Length),
  m_maxStartLine3Length(maxStartLine3Length),
  m_numHeaders(numHeaders),
  m_maxHeaderNameLength(maxHeaderNameLength),
  m_maxHeaderValueLength(maxHeaderValueLength),
  m_messageState(START_MESSAGE),
  m_pushHeaderState(START_LINE_TOKEN1),
  m_chunkSize(http::CHUNK_SIZE_LENGTH + 2), // this allows room for the trailing CRLF
  m_pAccessLog(pAccessLog),
  current_pipeline_data_(0),
  m_endlineBuf(&http::strings_.endline_),
  response_(cache,
            maxStartLine1Length, 
            maxStartLine2Length, 
            maxStartLine3Length,
            numHeaders),
  header_handler_(response_, streamType),
  request_postprocessor_(p_postprocessor)
{
}

HTTPProxyStreamHandler* HTTPProxyStreamHandler::clone()
{
  return new HTTPProxyStreamHandler( cache_,
                                     host_map_,
                                     m_streamType,
                                     m_maxStartLine1Length,
                                     m_maxStartLine2Length,
                                     m_maxStartLine3Length,
                                     m_numHeaders,
                                     m_maxHeaderNameLength,
                                     m_maxHeaderValueLength,
                                     m_pAccessLog,
                                     request_postprocessor_);
}

socketlib::STATUS HTTPProxyStreamHandler::processData(read_write_buffer& buf)
{
  //
  // This is a hack.  The proxylib is basically requesting a flush here.
  // If we are still buffering headers, we can't flush, so return
  // complete.  Probably should add a flush function to the API.
  if(buf.getWorkingLength() == 0 &&
     (m_messageState == PARSE_HEADER)){
    return socketlib::COMPLETE;
  }
  for(;;){
    switch(m_messageState){
      case START_MESSAGE:
        if(!set_current_pipeline_data()){
#warning not sure about this returnvalue
          if(m_streamType == http::REQUEST){
            return socketlib::INCOMPLETE;
          }
          else{
            return socketlib::COMPLETE;
          }
        }
        if(current_pipeline_data_->process_type_ == pipeline_data::DENY){
          error_response_.reset();
          header_pusher_.reset(error_response_.get_message_buffer(),
                               *GetProxyStreamInterface()->getDest());
          m_messageState = PUSH_HEADER;
        }
        else{
          m_messageState = PARSE_HEADER;
        }
        break;
        
      case PARSE_HEADER:  
        http::STATUS parseStatus;
        parseStatus = m_headerParser.parseHeaders(buf);
        if(parseStatus == http::COMPLETE){
          m_messageState = PUSH_HEADER;
          header_pusher_.reset(header_handler_.get_message_buffer(),
                               *GetProxyStreamInterface()->getDest());
          if(request_postprocessor_ && m_streamType == http::REQUEST){
            if(request_postprocessor_->process_request(
                 header_handler_.get_message_buffer())){
              logRequest();
              break;
            }
            else{
              parseStatus = http::INVALID;
            }
          }
          else{
            logRequest();
            break;
          }
          break;
        }
        if(parseStatus == http::INVALID || parseStatus == http::DATAOVERFLOW){
          if(m_streamType == http::REQUEST){
            current_pipeline_data_->process_type_ = pipeline_data::DENY;
            current_pipeline_data_->request_complete_ = true;
            log_info("invalid request");
          }
          else{
            log_info("invalid response");
          }
          return socketlib::DENY;
        }
        return socketlib::COMPLETE;

      case PUSH_HEADER:
        socketlib::STATUS pushStatus;
        pushStatus = header_pusher_.push_header();
        if(pushStatus == socketlib::COMPLETE){
          m_messageState = PARSE_BODY;
          current_pipeline_data_->request_complete_ = true;
          if(current_pipeline_data_->process_type_ == pipeline_data::DENY){
            //
            // Force the end of connection.
            return socketlib::SRC_CLOSED;
          }
          else if(m_streamType == http::RESPONSE && 
             current_pipeline_data_->process_type_ == pipeline_data::HEAD){
            // 
            // If the request was a HEAD request, override 
            // what the response headers claim that the body encoding
            // is and set the body encoding to none.  This shit
            // drives me nuts about the HTTP spec.
            m_bodyParser.reset(http::NONE, 0);
          }
          else{
            //
            // Just do the sensible thing, and get the body encoding
            // from the headers.
            m_bodyParser.reset(header_handler_.get_body_encoding(), 
                               header_handler_.get_message_size());
          }
          break;
        }
        return pushStatus;

      case PARSE_BODY:
        parseStatus = m_bodyParser.parseBody(buf);
        if(parseStatus == http::COMPLETE){
          GetProxyStreamInterface()->flush();
          complete_message();
          break;
        }
        if(parseStatus == http::INVALID ||
           parseStatus == http::DATAOVERFLOW){
          return socketlib::DENY;
        }
        //
        // This next state was causing a subtle edge-triggered state jam problem.  The problem
        // here is that INCOMPLETE can mean two different things.
        //
        // 1) The body isn't completed in the buffer, and I need to read again.
        // 2) The upstream server isn't ready to write, and I need a write event.
        //
        // What was happening is in the first case I am returning
        // incomplete here, which causing the proxyStream handler to wait
        // for another event, but there is data left to read in the socket,
        // and sense we are edge-triggered we don't get notified again, hence
        // the state-machine jams.
        //
        // The problem was fixed by adding the second WRITE_INCOMPLETE code.
        if(parseStatus == http::INCOMPLETE){
          return socketlib::COMPLETE;
        }
        if(parseStatus == http::WRITE_INCOMPLETE){
          return socketlib::INCOMPLETE;
        }
        if(parseStatus == http::IOFAILURE){
          return socketlib::DEST_CLOSED;
        }
        CHECK_CONDITION_VAL(false, "unknown return value from parseBody", parseStatus);
        break;
      default:
        CHECK_CONDITION_VAL(false, "unknown state in processData", m_messageState);
    }
  }
  //
  // Should return from inside loop
  CHECK_CONDITION(false, "fell out processData loop");
}

http::STATUS HTTPProxyStreamHandler::set_body(read_write_buffer& body, bool bComplete)
{
  //
  //
  socketlib::STATUS returnVal = GetProxyStreamInterface()->forward(body);

  return convert_proxy_status(returnVal);
}

void HTTPProxyStreamHandler::complete_message()
{
  clear_current_pipeline_data();
  initialize_state();
}

void HTTPProxyStreamHandler::reset()
{
  GetProxyStreamInterface()->get_pipeline_data_queue()->empty_queue();
  initialize_state();
}

void HTTPProxyStreamHandler::initialize_state()
{
  m_currentField = 0;
  m_currentDumpHeader = 0;
  m_messageState = START_MESSAGE;
  m_pushHeaderState = START_LINE_TOKEN1;
  m_headerParser.reset();
  m_bodyParser.reset(http::NONE, 0);
  m_chunkSize.reset();
  header_handler_.reset();
  
}


void HTTPProxyStreamHandler::set_body_encoding(http::BODY_ENCODING bodyEncoding)
{
  //
  // This is a no-op as we already know what the bodyEncoding is.
  // Not all IHTTPBodyReceivers may have that luxury.
  //
}


void HTTPProxyStreamHandler::set_chunk_size(unsigned int chunkSize)
{
  int chunkSizeChars = snprintf((char*)(&m_chunkSize.get_raw_buffer()[0]), m_chunkSize.getPhysicalLength(), "%X", chunkSize);
  m_chunkSize.set_working_length(chunkSizeChars + 2);
  m_chunkSize[chunkSizeChars] = http::CR;
  m_chunkSize[chunkSizeChars + 1] = http::LF;
  
  m_endlineBuf.setWritePosition(0);
}

http::STATUS HTTPProxyStreamHandler::forward_chunk_trailer()
{
  socketlib::STATUS returnVal = GetProxyStreamInterface()->forward(m_endlineBuf);
  return convert_proxy_status(returnVal);
}

http::STATUS HTTPProxyStreamHandler::forward_chunk_size()
{
  socketlib::STATUS returnVal = GetProxyStreamInterface()->forward(m_chunkSize);
  return convert_proxy_status(returnVal);
}


void HTTPProxyStreamHandler::logRequest()
{
  //
  // Don't bother to do anything if the log file
  // isn't open.  
  //
  if(!m_pAccessLog->isOpen()){
    return;
  }
  
  if(m_streamType == http::REQUEST){
    current_pipeline_data_->logrecord_.reset();
    copyRequestDataToLog(header_handler_.get_message_buffer(),
                         current_pipeline_data_->logrecord_);
  }
  else if(m_streamType == http::RESPONSE){

    copyResponseDataToLog(header_handler_.get_message_buffer(),
                          current_pipeline_data_->logrecord_);
    
    m_pAccessLog->logAccess(current_pipeline_data_->logrecord_);
    
  }
  else{
    assert(false);
  }
}
 
void HTTPProxyStreamHandler::copyRequestDataToLog(http::message_buffer& messageBuffer,
                                                  CombinedLogRecord& logRecord)
{
  http::HTTPRequestBufferWrapper bufferWrapper(messageBuffer);
  
  logRecord.remoteIP = GetProxyStreamInterface()->getSrcAddress();
  
  bufferWrapper.getMethod().appendToString(logRecord.requestline);
  logRecord.requestline += ' ';
  bufferWrapper.getURI().appendToString(logRecord.requestline);
  logRecord.requestline += ' ';
  bufferWrapper.getHTTPVersionBuffer().appendToString(logRecord.requestline);
  
  unsigned int headerIndex;
  if(bufferWrapper.getHeaderWithNameIndex("referer", headerIndex)){ 
    bufferWrapper.getFieldValueN(headerIndex).appendToString(logRecord.referer);
  }
  
  if(bufferWrapper.getHeaderWithNameIndex("user-agent", headerIndex)){ 
    bufferWrapper.getFieldValueN(headerIndex).appendToString(logRecord.userAgent);
  }
}

void HTTPProxyStreamHandler::copyResponseDataToLog(http::message_buffer& messageBuffer,
                                                   CombinedLogRecord& logRecord)
{
  
  http::HTTPResponseBufferWrapper bufferWrapper(messageBuffer);
  bufferWrapper.getStatusCode().appendToString(logRecord.status);
}

proxylib::IProxyStreamHandler::FORWARD_ADDRESS_STATUS HTTPProxyStreamHandler::getForwardAddressStatus()
{
  http::message_buffer& buffer = header_handler_.get_message_buffer();
  
  if(m_streamType != http::REQUEST){
    return proxylib::IProxyStreamHandler::STREAM_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }

  http::HTTPRequestBufferWrapper requestWrapper(header_handler_.get_message_buffer());
  if(requestWrapper.getHTTPVersion() == http::HTTPRequestBufferWrapper::HTTP1){
    return proxylib::IProxyStreamHandler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }
  
  unsigned int hostnameIndex;
  if(!buffer.get_header_index_by_name("Host", hostnameIndex)){
    return proxylib::IProxyStreamHandler::FORWARD_ADDRESS_NOT_YET_AVAILABLE;
  }

  std::string hostname;
  buffer.get_field_value(hostnameIndex).appendToString(hostname);

  std::map<std::string, std::pair<httplib::URL, bool> >::const_iterator it;

  it = host_map_.find(hostname);
  if(it == host_map_.end()){
    return proxylib::IProxyStreamHandler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
  }

  m_forwardAddress = *it->second.first.endpoint.data();
  std::string base_path = it->second.first.path;
  buffer.get_status_line_2().appendToString(base_path);
  buffer.get_status_line_2().appendFromString(base_path.c_str());
  if(!it->second.second){
    buffer.get_field_value(hostnameIndex).appendFromString(it->second.first.hostname.c_str());
  }
  return proxylib::IProxyStreamHandler::FORWARD_ADDRESS_AVAILABLE;

#ifdef NOTUSINGJITCONNECT
  return proxylib::IProxyStreamHandler::CONNECTION_DOES_NOT_PROVIDE_FORWARD_ADDRESS;
#endif  
  
}


sockaddr& HTTPProxyStreamHandler::getForwardAddressFromStream()
{
  return m_forwardAddress;
}


bool HTTPProxyStreamHandler::set_current_pipeline_data()
{
  if(m_streamType == http::REQUEST){
    if(GetProxyStreamInterface()->get_pipeline_data_queue()->queue_full()){
      return false;
    }
    current_pipeline_data_ =
      boost::polymorphic_downcast<pipeline_data*>(GetProxyStreamInterface()->get_pipeline_data_queue()->queue_element());
  }
  else if(m_streamType == http::RESPONSE){
    if(GetProxyStreamInterface()->get_pipeline_data_queue()->queue_empty()){
      return false;
    }
    else{
      current_pipeline_data_ =
      boost::polymorphic_downcast<pipeline_data*>(GetProxyStreamInterface()->get_pipeline_data_queue()->front());
      if(!current_pipeline_data_->request_complete_){
        return false;
      }
      GetProxyStreamInterface()->get_pipeline_data_queue()->dequeue_element();
    }
  }
  else{
    assert(false);
  }
  return true;
}

void HTTPProxyStreamHandler::clear_current_pipeline_data()
{
  current_pipeline_data_ = 0;
}

http::STATUS HTTPProxyStreamHandler::convert_proxy_status(socketlib::STATUS status)
{
  if(status == socketlib::DEST_CLOSED){
    return http::IOFAILURE;
  }
  else if(status == socketlib::COMPLETE){
    return http::COMPLETE;
  }
  else if(status == socketlib::INCOMPLETE){
    return http::INCOMPLETE;
  }
  else if(status == socketlib::SRC_CLOSED){
    return http::IOFAILURE;
  }
  else if(status == socketlib::DENY){
    return http::INVALID;
  }
  else{
    CHECK_CONDITION_VAL(false, "invalid status", status);
  }
  //
  // return something to avoid warning.
  //
  return http::INVALID;
}
