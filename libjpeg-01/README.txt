This is a short documentation for the wrapper of the libjpeg library.

libjpeg is stored in the repo at ext_src/libJpeg and was written by the Independed JPEG Group, http://www.ijg.org/.


Building the JavaScript library
===============================

1. Go to ext_src/libJpeg
 1.1 emconfigure ./configure --disable-shared
 1.2 emmake make clean
 1.3 emmake make

2. Go to src
 2.1 ./makeEmcc

Libary DecodeImage.js is built now, open loadJpeg.html to test the library.

Known Issues:
- Some JPEG-files cannot be successfully loaded
- Compiler optimizations are disabled, so performance is not that good



Building the native test program
================================

1. Go to ext_src/libJpeg
 1.1 ./configure
 1.2 make && make install

2. Go to compilable_c_src
 2.1 make

Test program is built now. Note that the class file DecodeImage.cpp contains a main method for testing reasons.


