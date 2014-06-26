#!/bin/bash

export ARCH="-arch i386 -arch x86_64"
export CFLAGS="${ARCH} -mmacosx-version-min=10.7 -Wno-unused-function"
export CXXFLAGS="${CFLAGS}"
export LDFLAGS="${ARCH} -mmacosx-version-min=10.7"

#set -e

here=`pwd`
for d in cmt/src FIL mcp tap/tap-plugins swh; do
  cd $d
  make -f Makefile.osx
  cd "${here}"
done

rm -rf build
mkdir -p build

for f in `find . -name '*.so'`; do cp $f build/`basename $f`; done

files=`find . -name '*.dylib'`
for file in $files; do
  cp "${file}" "build/`basename ${file%\dylib}so`"
done
