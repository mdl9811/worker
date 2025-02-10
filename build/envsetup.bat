@REM Copyright (c) 2024 mdl. All rights reserved.
@echo off
setlocal
cd /d %~dp0..

set LLVM_BIN=C:\Program Files\LLVM\bin
set ARCH=x86_64
set TARGET=windows
set EXE=.exe
set HOST=unix
set HOST_EXE=.exe

if "%1" == "windows" set TARGET=windows
if "%1" == "uinx" set TARGET=uinx
if "%2" == "--debug" set  BUILD=debug
if "%2" == "--release" set  BUILD=release
if not exist out mkdir out

(
  echo LLVM_BIN=%LLVM_BIN%
  echo ARCH=%ARCH%
  echo TARGET=%TARGET%
  echo BUILD=%BUILD%
  echo EXE=%EXE%
  echo HOST=%HOST%
  echo HOST_EXE=%HOST_EXE%
) > out\args.ninja
echo %TARGET% %BUILD%