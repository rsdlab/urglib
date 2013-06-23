#!/bin/bash

mkdir -p doc_inc
cp include/libkobuki.h doc_inc/
cp include/Kobuki.h doc_inc/
doxygen Doxyfile

mkdir -p build-linux
cd build-linux
cmake ../
make
cpack -G DEB -D CPACK_PACKAGE_CONTACT=staff@rt-net.jp ../
#cpack -G PackageMaker -D CPACK_PACKAGE_CONTACT=staff@rt-net.jp ../
cd ../
