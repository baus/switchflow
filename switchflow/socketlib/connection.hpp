#ifndef SOCKETDATA_H
#define SOCKETDATA_H

#include <boost/config.hpp>

#ifndef BOOST_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <windows.h>
#endif

#include <sys/types.h>

#include <string>

#include <util/read_write_buffer.hpp>

#include "status.hpp"

namespace socketlib{
  
  class connection{
  public:
    //
    // Connection State
    // for more information on partially connected sockets see the shutdown() docs.
    enum STATE{
      NOT_CONNECTED, // the socket hasn't been connected 
      CONNECTED,     // the socket is currently connected 
      CONNECTING,    // the socket is the in the process of connecting (ie
                     //   connect was called but hasn't been notifed with a POLLOUT)
                     
      READ_SHUT,     // the socket was fully connected, but now the read line is closed
      WRITE_SHUT,    // the socket was fully connected, but now the write line is closed
      HUNGUP         // the socket was connected, but now it is disconnected 
    };
    
    ///
    // Construct the socket data
    //
    // @param bufferLength the length of the input buffer
    connection(unsigned int buffer_length);

    void set_nonblocking();
    
    ///
    /// reset to defaults
    void reset();

    bool ready_to_read();

    bool ready_to_write();

    bool open_for_read();
    bool open_for_write();

    void cork();    
    void uncork();

    STATE state();
    void state(STATE new_state);

    read_write_buffer& readBuffer();

    int fd();
    void fd(int newFd);

    void flush();

    ///
    /// Get non-resolved IP address of socket.
    ///
    /// @return static pointer to string.  Callers
    /// must make a copy of the string.  This is
    /// returned via a char* to reduce unnecessary
    /// string copies.
    const char* get_ip_addr();

    //
    // These should be private. Need to rethink event notification here.
    void ready_to_read(bool bValue);
    void ready_to_write(bool bValue);

    //
    // Perform a non-blocking read to the buffer associated
    // with the connection.  If the socket isn't ready to read, it
    // returns immediately.  It also set the state of
    // src appropriately.
    STATUS non_blocking_read();

    //
    // Perform a non-blocking write from the giving buffer.  If
    // the dest isn't ready to write the function returns immediately.
    // Also sets the state of the buffer and dest appropriately.
    // callers should take careful note of the return values of this function.
    //
    // @param dest information for the socket to which data will be written
    // @param buf the buffer to write from
    //
    // @return SUCCESS     The operation completed without incident.  This could
    // simply mean that socket isn't ready to write.  Do not assume
    // anything happened.<BR><BR>
    // DEST_CLOSED The destination closed during or prior to writing.
    // This can happen if the socket is hungup after it is notified
    // that it can read, but before the write is attempted.  The
    // handling of this situation probably depends on if the function
    // is attempting to write to the server or the client.<BR><BR>
    // FATAL_ERROR A significant problem occured while writing.  It would probably
    // be best to shutdown if this is returned, as an unexpected
    // event has occured at the OS level.
    STATUS non_blocking_write(read_write_buffer& buf);

  private:
    void cork(int state);


    //
    // Is the socket ready to read w/out blocking?
    bool ready_to_read_;

    //
    // Is the socket ready to write w/out blocking?
    bool ready_to_write_;

    //
    // file descriptor of the socket
    int fd_;

    //
    // buffer to read into
    read_write_buffer buffer_;

    //
    // The conection state
    STATE state_;

    //
    // string non-resolved ip address (example: "192.168.0.1")
    std::string ip_addr_;

  };


    
} // namespace socketlib

#endif // SOCKETDATA_H
