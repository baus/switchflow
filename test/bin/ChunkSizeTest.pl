#!/usr/bin/perl
#
# I'm putting extra verbose comments here, since I don't know
# much about Perl and this will help me remember the syntax when
# I look at it again 6 months from now.  
#
# I'm not using much error checking, since this is a test program.
# I'll leave the error checking to the product code. 
#

#
# Force strict syntax checking.
#
use strict;

use lib::HTTPTestLib;
my $method;
my $host;
my $path;
my $port;

($host, $path, $port, $method) = lib::HTTPTestLib::parseCommandline(@ARGV);

syswrite(STDOUT, "host: $host\n");
syswrite(STDOUT, "port: $port\n");
syswrite(STDOUT, "path: $path\n");
syswrite(STDOUT, "method: $method\n");

my $inputbuffer;
my $CRLF = $HTTPTestLib::CRLF;

my $sock = lib::HTTPTestLib::connect($host, $port);

#
# This is the HTTP startline. 
#
# The first CRLF signifies the end of the startline.
# The second CRLF signifies the end of the field values.
# A CRLF on line by itself signifies the end of the field values.
# In this case it signifies that there are no field headers.
#
#
my $startline = "$method $path HTTP/1.0$CRLF$CRLF";

#
# Print the HTTP startline so we can see what we are doing.
#
syswrite(STDOUT, $startline, length($startline));

#
# Send the HTTP startline to the server
#
syswrite($sock, $startline, length($startline));

#
# sysread will return between 0 and the size of the buffer
# specified in the call.  If it returns 0, it means the 
# connection has been closed by the peer.    
#
my $bytesread;

#
# This will read until end of file.  If server doesn't close
# connection 
#
while($bytesread = sysread($sock, $inputbuffer, 1500)){
  #
  # write the results of the request to stdout for observation.
  #
  syswrite(STDOUT, $inputbuffer, $bytesread); 
}

close(SOCK);
