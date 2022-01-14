#!/bin/sh
unpack(){
 	git clone https://github.com/Butterzz1288/ImgView.git --depth 1 lemon-gnuboy
 	export BUILD_DIR=ImgView
}
 
buildp(){
 	cd $BUILD_DIR

	./configure --host=x86_64-lemon
 	make -j"$JOBCOUNT"
 	cp lemongnuboy "$LEMON_SYSROOT/$LEMON_PREFIX/bin/ImgView.lef"
}
