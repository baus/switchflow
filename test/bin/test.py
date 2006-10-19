import HTTPTest

httptest = HTTPTest.httptest()

httptest.connect("localhost", 80)

print "start content length body test..."
httptest.contentLengthBodyTest()
print "end content length body test..."

print "fragmented status test start..."
httptest.fragmentedStatusLine()
print "fragmented status test done..."

httptest.versionOverrun()

#
httptest.queryRequest("INVALIDMETHOD")
#
httptest.queryRequest("12345")

#
httptest.queryRequest("GET", "/123456789")

#
httptest.queryRequest("GET", "/1234567890")
#
httptest.queryRequest()

#
httptest.queryRequest("GET", "/", "HTTP/1.0", 2)
#
httptest.queryRequest("GET", "/", "HTTP/1.0", 3)
#
httptest.malformedHeader()
#
httptest.queryRequest("GET", "/", "HTTP/1.0", 400)

httptest.post("localhost", 10001)


