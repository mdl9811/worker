#!/bin/bash

#
# Copyright (c) 2024 mdl. All rights reserved.
#

LLVM=/usr/lib/llvm-19
ARCH=x86_64
TARGET=unix
EXE=
HOST=unix
HOST_EXE=
BUILD=release

usage() {
  echo "build/envsetup.sh [--llvm=<LLVM>] [--<release|debug>]"
  echo " Usage:"
  echo "  --llvm=<LLVM> - LLVM path (default: /usr/lib/llvm-19)"
  echo "  --<release|debug> - build type (default: release)"
  exit
}

for V in "$@"; 
do
  IFS='='; set -- $V
  case $1 in
    windows) 
    TARGET=windows 
    ;;
    --debug) 
    BUILD=debug 
    ;;
    --release) 
    BUILD=release 
    ;;
    --llvm) 
    LLVM_BIN=$2 
    ;;
    --help) 
    HOST_EXE=$2 
    ;;
  esac
done

if !([ $BUILD == "release" ] || [ $BUILD == "debug" ]); then
>&2 echo "Error: invalid BUILD type $BUILD."
exit
fi

if [ ! -e "$LLVM" ]; then
>&2 echo "Invalid LLVM $LLVM."
exit
fi

MSG="Setting up build for $TARGET $BUILD"

echo $MSG
mkdir -p out
mkdir -p out/${TARGET}/${ARCH}/${BUILD}/lib
echo -e "LLVM=${LLVM}\nARCH=${ARCH}\nTARGET=${TARGET}\nBUILD=${BUILD}\nEXE=${EXE}\nHOST=${HOST}\nHOST_EXE=${HOST_EXE}" > out/args.ninja
ninja -t recompact >/dev/null 2>&1
