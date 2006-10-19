#!/usr/bin/perl
package lib::HTTPTestLib;
use strict;
use URI::URL;

#
# I'm using the low level BSD socket functionality, and 
# this line includes that module. 
#
use Socket;

use vars qw(@ISA @EXPORT $VERSION);

use Exporter;
$VERSION = 0.01;
@ISA = {'Exporter'};
@EXPORT = qw($CRLF, &parseCommandline, &connectToHost) ;


#
# It is important to use the actual values for 
# CR and LF.  \r \n isn't portable.  
# 
# Using "" instead of '' allows the use of escape characters.
#
$HTTPTestLib::CRLF = "\015\012";

sub parseCommandline
{
  my (@commandline) = @_;
  #
  # Shift pops an element off the front of an array and
  # advances the pointer to the front.  By default it 
  # uses shifts the command line argument array, which
  # is useful for basic command line parsing.  The first
  # command line option should be the URL. 
  #
  my $urlstring = shift(@commandline);

  #
  # Shift the elements again, this time getting the HTTP method (GET, POST, HEAD, etc.)
  # This doesn't check that this is a valid HTTP method. This could be useful for trying
  # to send request with bogus HTTP methods. 
  #
  my $method = shift(@commandline);
  
  #
  #
  #
  my $numheaders = shift(@commandline);
  
  #
  # These variables will contain the parsed values used to make the
  # connection and HTTP request.
  #
  my $host;
  my $path;
  my $port;
  
  #
  # If the urlstring is empty, use some defaults 
  #
  if ($urlstring eq '')
  {
    #
    # use low level I/O calls.  I don't want to get confused with
    # Perl's built in buffer routines so using the sys* calls avoids
    # the buffering routines, and calls the OS directly after converting
    # to the correct C types.
    #
    syswrite(STDOUT, "found no URL.  Using http://localhost:8080/\n");
    #
    # Use these as defaults
    #
    $host = "127.0.0.1";
    $path = "/";
    $port = 8080;
  }
  else{
    #
    # Use the LWP to parse the string.  
    # This is done by instanciating a URI::URL with the 
    # the string we read in from the command line.  
    #
    my $url = new URI::URL($urlstring);
    #
    # copy the parsed text back to the following strings.
    #
    $host = $url->host();
    $path = $url->full_path();
    $port = $url->port();
  }
  if($method eq '')
  {
    syswrite(STDOUT, "found no HTTP method.  Using GET\n");
    $method = "GET";
  }
  if($numheaders eq '')
  {
      syswrite(STDOUT, "found no numheaders.  Using 0\n");
      $numheaders = 0;
  }
  return ($host, $path, $port, $method, $numheaders); 
};

sub connect
{
  my ($host, $port) = @_;
  
  my $packed_addr = inet_aton($host);
  my $destination = sockaddr_in($port, $packed_addr);
  my $protocol = getprotobyname('tcp');


  #
  # The SOCK variable is declared here.  In Perl,
  # a variable without a $ signifies a file descriptor.  
  #
  # I'm not exactly sure why they do this, but my guess is
  # it is for built in operators like <> which operate on file
  # descriptors.
  #
  my $sock;
  socket($sock, PF_INET, SOCK_STREAM, $protocol) or
      die "Could not create socket: $!\n";

  connect($sock, $destination) or 
      die "Could not connect socket: $!\n";

  return ($sock);
}

1;
