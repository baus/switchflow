rm -f $TALLAC/thirdparty/lib/debug/*
rm -f $TALLAC/thirdparty/lib/release/*




pushd $TALLAC

pushd switchflow
sh buildthirdparty.sh
popd

sh ./configbuild.sh
cd $TALLAC/sfrp-release
make clean
make
cd $TALLAC/sfrp-debug
make clean
make

popd
