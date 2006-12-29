#!/bin/sh

DIRNAME=switchflow-reverse-proxy-`date +%m%d%Y`

cd $TALLAC
rm -f switchflow-reverse-proxy*.tgz
rm -rf switchflow-reverse-proxy*
mkdir $DIRNAME
cd $TALLAC/release/sfrp
make clean
make 
cp $TALLAC/release/sfrp/app/sfrp $TALLAC/$DIRNAME
cp $TALLAC/sfrp/docs/README $TALLAC/$DIRNAME
cp $TALLAC/sfrp/docs/sfrp.conf $TALLAC/$DIRNAME
$TALLAC/$DIRNAME/sfrp --version
cd $TALLAC

OS=`uname -s`
if [ "$OS" = "FreeBSD" ]; then
TARFILE=switchflow-reverse-proxy-`$TALLAC/$DIRNAME/sfrp --version`-`uname -s`-`uname -m`-`date +%m%d%Y`.tgz
TARCMD="tar -c -z -f $TARFILE $DIRNAME"
fi
if [ "$OS" = "Linux" ]; then
TARFILE=switchflow-reverse-proxy-`$TALLAC/$DIRNAME/sfrp --version`-`uname -s`-`uname -i`-`date +%m%d%Y`.tgz
TARCMD="tar cz $DIRNAME -f $TARFILE"
fi

echo building $TARFILE

$TARCMD
