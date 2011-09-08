@echo off

:: Copyright (c) 2011 The Native Client Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

setlocal

set NACL_SDK_ROOT=%~dp0..\..\..\..
:: NACL_TARGET_PLATFORM is really the name of a folder with the base dir -
:: usually nacl-sdk-root - within which the toolchain for the target platform
:: are found.
set NACL_TARGET_PLATFORM=.

:: Set the PYTHONPATH and SCONS_LIB_DIR so we can import SCons modules
set SCONS_LIB_DIR=%~dp0third_party\scons-2.0.1\engine
set PYTHONPATH=%NACL_SDK_ROOT%\third_party\scons-2.0.1\engine

:: We have to do this because scons overrides PYTHONPATH and does not preserve
:: what is provided by the OS.  The custom variable name won't be overwritten.
set PYMOX=%NACL_SDK_ROOT%\third_party\pymox

:: Run the included copy of scons.
python -O -OO "%NACL_SDK_ROOT%\third_party\scons-2.0.1\script\scons" ^
--warn no-visual-c-missing ^
--file=build.scons ^
--site-dir="%NACL_SDK_ROOT%\build_tools\nacl_sdk_scons" %*
