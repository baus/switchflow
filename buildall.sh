rm -f $TALLAC/thirdparty/lib/debug/*
rm -f $TALLAC/thirdparty/lib/release/*




pushd $TALLAC

pushd switchflow
sh buildthirdparty.sh
popd

sh ./configbuild.sh
cd $TALLAC/release/sfrp
make clean
make
cd $TALLAC/debug/sfrp
make clean
make

popd
