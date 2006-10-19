import socket
import urllib
import httplib
import time
import sys

class httptest:
    def __init__(self):
        pass
    
    def connect(self, host, port):
        self.host = host
        self.port = port

    def _connect(self, host, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((self.host, self.port))
        return sock
                     
    def send(self, sock, msg):
        sock.sendall(msg)

    def receive(self, sock):
        msg = ''
        while True:
            chunk = sock.recv(1500)
            if chunk == '':
                return msg
            msg = msg + chunk

    def createStartLine(self, method, uri, version):
        return method + " " + uri + " " + version

    def versionOverrun(self, method="GET", uri="/", overrunLength = 100):
        try:
            sock = self._connect(self.host, self.port)
            startline = self.createStartLine(method, uri, "HTTP/1.0")
            self.send(sock, startline)
            i = 0
            while i < overrunLength:
                self.send(sock, str(i))
                i = i + 1
            
            self.send(sock, "\r\n")
            self.send(sock, "\r\n")
        except socket.error:
            "error occured trying to overrun method"

    def fragmentedStatusLine(self):
        try:
            sock = self._connect(self.host, self.port)
            statusLine = self.createStartLine("GET", "/", "HTTP/1.0")
            for c in statusLine:
                # Send each character in its own buffer to test segmented parsing code.
                sock.send(c)
                sys.stdout.write(c)
                sys.stdout.flush()
                time.sleep(1)
                
            testheader = "\r\nx-summitsage-test-header: this is a test header\r\n\r\n"
            for c in testheader:
                sock.send(c)
                sys.stdout.write(c)
                sys.stdout.flush()
                time.sleep(1)
                    
        except socket.error:
            print "socket error sending fragmented request"
            return

        try:
            response = httplib.HTTPResponse(sock, 0, True)
            response.begin()
            response.read()
            print "Status: " + str(response.status)

        except socket.error:
            print "socket error reading response from fragmented request"

        
    def malformedHeader(self,
                        method="GET",
                        uri="/",
                        version = "HTTP/1.0",
                        numHeaders = 1):
        try:
            sock = self._connect(self.host, self.port)
            startline = self.createStartLine(method, uri, version)
            testheader = "fooed-up-header"
            print testheader
            self.send(sock, startline)
            self.send(sock, "\r\n")
            self.send(sock, testheader)
            self.send(sock, "\r\n")
            self.send(sock, "\r\n")
            response = httplib.HTTPResponse(sock, 0, True)
            response.begin()
            response.read()
            print "Status: " + str(response.status)

        except socket.error:
            print "socket error in malformed header"
            return 

        except httplib.BadStatusLine:
            print "BadStatusLine error in malformed header"
            return
        
        sock.close()
            
    def contentLengthBodyTest(self, body="some data", method="GET", uri="/", version = "HTTP/1.0"):
        try:
            sock = self._connect(self.host, self.port)
            startline = self.createStartLine(method, uri, version)
            self.send(sock, startline)
            self.send(sock, "\r\n")
            contentLength = "content-length: " + str(len(body)) + "\r\n\r\n"
            print contentLength,
            self.send(sock, contentLength)
            for c in body:
                sock.send(c)
                time.sleep(1)
                sys.stdout.write(c)
                sys.stdout.flush()

        except socket.error:
            print "socket error occured during content length test."


        try:
            response = httplib.HTTPResponse(sock, 0, True)
            response.begin()
            response.read()
            print "Status: " + str(response.status)

        except socket.error:
            print "socket error reading content length test"

        except httplib.BadStatusLine:
            print "content length test response had bad status line."

        except IOError:
            print "an IOError occured"


    def queryRequest(self, method="GET", uri="/", version = "HTTP/1.0", numHeaders = 1):
        try:
            sock = self._connect(self.host, self.port)
            startline = self.createStartLine(method, uri, version)
            testheader = "x-summitsage-test-header: this is a test header\r\n"
            self.send(sock, startline)
            self.send(sock, "\r\n")
            print startline + " " + str(numHeaders)
            while numHeaders > 0:
                self.send(sock, testheader)
                numHeaders = numHeaders - 1
            self.send(sock, "\r\n")
                
        except socket.error:
            print "socket error sending queryRequest"
            return
            
        try:
            response = httplib.HTTPResponse(sock, 0, True)
            response.begin()
            response.read()
            print "Status: " + str(response.status)

        except socket.error:
            print "socket error reading queryRequest"

        except httplib.BadStatusLine:
            print "response had bad status line."

        except IOError:
            print "an IOError occured"
            
        sock.close()

    def post(self, host, port=80):
        """Post a simple request to the given host"""
        print "POST test"
        params = urllib.urlencode({'spam': 1, 'eggs': 2, 'bacon': 0})
        headers = {"Content-type": "application/x-www-form-urlencoded",
                   "Accept": "text/plain"}
        conn = httplib.HTTPConnection(host, port)
        conn.request("POST", "/", params, headers)
        response = conn.getresponse()
        
        print "Status: " + str(response.status)
        data = response.read()
        conn.close()
