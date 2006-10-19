#!/bin/sh
pushd $TALLAC/test/bin 

echo "running sanity check test"
perl QueryRequestTest.pl http://localhost:10001/
echo "running method length test"
perl QueryRequestTest.pl http://localhost:10001/ INVALIDMETHOD
echo
echo
echo "running method length test 2"
perl QueryRequestTest.pl http://localhost:10001/ 12345
echo
echo
echo "running URI length test -- this should give file not found"
perl QueryRequestTest.pl http://localhost:10001/123456789
echo
echo
echo "running URI length test -- should deny"
perl QueryRequestTest.pl http://localhost:10001/1234567890
echo
echo
echo "re-running sanity check test"
perl QueryRequestTest.pl http://localhost:10001/
echo
echo
echo "running number of headers test -- should accept"
perl QueryRequestTest.pl http://localhost:10001/ GET 2
echo
echo
echo "running number of headers test 2 -- should deny"
perl QueryRequestTest.pl http://localhost:10001/ GET 21
echo
echo
echo "running post test"
python post.py localhost 10001
popd
