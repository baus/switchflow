import sys
import httplib
import urllib

def post(host, port=80):
    """Post a simple request to the given host"""
    params = urllib.urlencode({'spam': 1, 'eggs': 2, 'bacon': 0})
    headers = {"Content-type": "application/x-www-form-urlencoded",
               "Accept": "text/plain"}
    print type(port)
    conn = httplib.HTTPConnection(host, port)
    conn.request("POST", "/", params, headers)
    response = conn.getresponse()
    print response.status, response.reason
    data = response.read()
    print data
    conn.close()

def main(argv=None):
    if argv is None:
        argv = sys.argv
    print sys.argv
    if len(argv) < 2 or len(argv) > 3:
        print >>sys.stderr, "wrong number of arguments"
        return 2
    if len(argv) == 2:
        post(argv[1])
    if len(argv) == 3:
        post(argv[1], argv[2])

if __name__ == "__main__":
    sys.exit(main())


