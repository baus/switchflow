//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (C) Christopher Baus.  All rights reserved.

// system includes 
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <errno.h>

// c++ includes
#include <assert.h>
#include <string>
#include <iostream>

// library includes
#include <util/logger.hpp>

// local includes
#include "connection.hpp"

namespace socketlib{
  
  connection::connection(unsigned int bufferLength):
    ready_to_read_(false),
    ready_to_write_(false),
    fd_(-1),
    buffer_(bufferLength),
    state_(NOT_CONNECTED)
  {  
    ip_addr_.resize(11);
  }
    
  void connection::reset()
  {  
    state_ = NOT_CONNECTED;
    buffer_.reset();
    fd_ = -1;
    ip_addr_ = "";
    ready_to_read_ = false;
    ready_to_write_ = false;
  }
    
  void connection::ready_to_read(bool bValue)
  {
    ready_to_read_ = bValue;
  }

  bool connection::ready_to_read()
  {
    return ready_to_read_ && open_for_read();
  }

  void connection::ready_to_write(bool bValue)
  {
    ready_to_write_ = bValue;
  }

  bool connection::ready_to_write()
  {

    return ready_to_write_ && open_for_write();
  }

  connection::STATE connection::state()
  {
    return state_;
  }

  void connection::state(STATE newState)
  {
    if(newState == WRITE_SHUT && state_ == READ_SHUT){
      state_ = HUNGUP;
    }
    if(newState == READ_SHUT && state_ == WRITE_SHUT){
      state_ = HUNGUP;
    }
    else{
      state_ = newState;
    } 
  }
  
  read_write_buffer& connection::readBuffer()
  {
    return buffer_;
  }

  void connection::fd(int newFd)
  {
    fd_ = newFd;
    if(newFd != -1){
      //
      // for edge triggered these should be
      // true.  For level triggered they should be
      // false.
      ready_to_read_ = false;
      ready_to_write_ = false;
      cork(1);
    }
  }
  
  int connection::fd()
  {
    return fd_;
  }
    
  const char* connection::get_ip_addr()
  {
    struct sockaddr_in peername;
    socklen_t peerlen=sizeof(struct sockaddr_in);
    int returnVal = getpeername(fd_, (struct sockaddr*)&peername, &peerlen);

    // assert(returnVal != -1);
    //
    // I wanted to assert here, but there seems to be an issue that
    // is undocumented.  When making an asynchronous connection, there is
    // a valid fd which is held in fd_, but the socket isn't connected yet.
    // in this case getpeername returns -1.  This is forcing me to rethink
    // the strategy of getting the peername as soon as the fd_ is set into
    // the socketData structure.

    if(returnVal == -1){
      return 0;
    }
    
    //
    // convert peername.sin_addr.s_addr to string here.
    return inet_ntoa(peername.sin_addr);
  }

  bool connection::open_for_read()
  {
    return (state_ == CONNECTED || state_ == WRITE_SHUT);
  }

  bool connection::open_for_write()
  {
    return (state_ == CONNECTED || state_ == READ_SHUT);
  }
  
  void connection::flush()
  {
    cork(0);
    cork(1);
  }  
  
  void connection::cork(int state)
  {
#if defined(TCP_NOPUSH)
    setsockopt(fd_, IPPROTO_TCP, TCP_NOPUSH, &state, sizeof(state));
    state = ~state;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));
#endif
#if defined(TCP_CORK) && !defined(TCP_NOPUSH)
    setsockopt(fd_, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
#endif
#if !defined(TCP_CORK) && !defined(TCP_NOPUSH)
#warning no way to buffer data into single packets.  Performance might suck ass.
#endif
  }

  void connection::uncork()
  {
    cork(0);
  }

  void connection::cork()
  {
    cork(1);
  }

  void connection::set_nonblocking()
  {
    int flags = O_RDWR | O_NONBLOCK | O_ASYNC;

    if (fcntl(fd_, F_SETFL, flags) < 0) {
      log_info("fcntl() returns error");
      return;
    }
  }

  STATUS connection::non_blocking_read()
  {
    if(!ready_to_read()){
      return socketlib::COMPLETE;
    }
    if(!readBuffer().fullyWritten()){
      //
      // It is tempting to buffer up some more data here, but why bother?  It could
      // lead to problems like non-contiguous buffers.  It would cause more problems
      // than it would solve.
      // Just let the OS buffer it until we are ready to write.
      return socketlib::COMPLETE;
    }
    readBuffer().reset();
    int physicalLength = readBuffer().getPhysicalLength();
    int retVal = ::read(fd(), &(readBuffer().get_raw_buffer()[0]), readBuffer().getPhysicalLength());
    if(retVal != -1){
      //
      // got some of the finest data available...
      if(retVal == 0){
        state(socketlib::connection::READ_SHUT);
        return socketlib::SRC_CLOSED;
      }
      readBuffer().set_working_length(retVal);
    }
    else{
      int localerrno = errno;
      if(errno == EINTR){
        //
        // Not sure what to do here.  One option is to return to the poll loop,
        // the other is to retry the operation.  For now log it, and continue.
        log_info("read was interrupted by signal");
        return socketlib::COMPLETE;
      }
      else if(errno == EAGAIN){
        //
        // can't read anymore with out blocking.  Normal condition.
        // need to set the state and continue out of function.
        ready_to_read(false);  
      }
      else if(errno == ECONNRESET){
        state(socketlib::connection::READ_SHUT);
        return socketlib::SRC_CLOSED;
      }
      else{
        //
        // Unknown error.  Shut the socket down to
        // reading.  The higher layers should shutdown
        // the connection in this situation.
        log_info("read failed with errno: ", errno);
        state(socketlib::connection::READ_SHUT);
        return socketlib::SRC_CLOSED;
      }
    }
    return socketlib::COMPLETE;
  }  

STATUS connection::non_blocking_write(read_write_buffer& buf)
{
  //
  // Continue until one of the following occurs:
  //
  // 1) all data is written
  // 2) an error occurs
  // 3) no more data could be written w/ out blocking.
  for(;;){
    if(!ready_to_write()){
      return INCOMPLETE;
    }   
    int startPosition = buf.getWritePosition();
    int numBytesToWrite = buf.getWriteEndPosition() - startPosition;
    //
    // Sanity check the hell out the buffer positions.  There would be nothing worse than
    // blowing the buffer here. 
    assert(numBytesToWrite >= 0);
    assert(numBytesToWrite <= buf.getWriteEndPosition());
    assert(numBytesToWrite <= buf.getWorkingLength());
    assert(buf.getWorkingLength() <= buf.getPhysicalLength());
    assert(startPosition >= 0);
    if(buf.getWorkingLength() == 0 || numBytesToWrite == 0){
      return COMPLETE;
    }
    assert(startPosition < buf.getWorkingLength());
    if(numBytesToWrite == 0){
      return COMPLETE;
    }
    int retVal = ::write(fd(), &(buf.get_raw_buffer()[startPosition]), numBytesToWrite);
    if(retVal != -1){
      buf.setWritePosition(startPosition + retVal);
      if(retVal >= numBytesToWrite){
        return COMPLETE;
      }
      //
      // continue through loop and attempt to write again...
    }
    else{
      CHECK_CONDITION_VAL(errno != EBADF, "write failed on unknown FD", fd());

      if(errno == EINTR){
        //
        // Not sure what to do here.  One option is to return to the poll loop,
        // the other is to retry the operation.  For now log it, and continue to try it again.
        log_info("write was interrupted by signal");
      }
      else if(errno == EAGAIN){
        //
        // can't write anymore with out blocking.  Normal condition.
        // need to set the state and continue out of function.
        ready_to_write(false);
        return INCOMPLETE;
      }
      else if( errno == EPIPE || errno == ECONNRESET ) {
        //
        // this may happen if the socket hangsup after it notifies it is available to
        // write.  calling functions should be prepared to do something useful here.
        state(connection::WRITE_SHUT);
        return DEST_CLOSED;
      }
      else{
        //
        // Unknown error.  Shut the socket down to
        // writing.  The higher layers should shutdown
        // the connection in this situation.
        log_info("write failed.  errno", errno);
        state(connection::WRITE_SHUT);
        return DEST_CLOSED;
      }
    }
  }
}

//

} //namespace proxylib
