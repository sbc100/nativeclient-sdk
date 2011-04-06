@echo off

:: Copyright (c) 2011 The Native Client Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

setlocal

set NACL_SDK_ROOT=%~dp0

:: If this batch file is run directly thru cygwin, we will get the wrong
:: version of python. To avoid this, if we detect cygwin, we need to then
:: invoke the shell script which will then re-invoke this batch file with
:: cygwin stripped out of the path.
:: Detect cygwin by trying to run bash.
bash --version >NUL 2>&1
if %ERRORLEVEL% == 0 (
    bash "%~dp0\scons" %* || exit 1
    goto end
)

:: Preserve a copy of the PATH (in case we need it later, mainly for cygwin).
set PRESCONS_PATH=%PATH%

:: Set the PYTHONPATH so we can import SCons modules
set PYTHONPATH=%~dp0third_party\scons-2.0.1\engine

:: We have to do this because scons overrides PYTHONPATH and does not preserve
:: what is provided by the OS.  The custom variable name won't be overwritten.
set PYMOX=%~dp0third_party\pymox

:: Add python, gnu_binutils and mingw to the path
set PATH=%~dp0third_party\python_26;%PATH%

:: Stop incessant CYGWIN complains about "MS-DOS style path"
set CYGWIN=nodosfilewarning %CYGWIN%

:: Run the included copy of scons.
python -O -OO "%~dp0third_party\scons-2.0.1\script\scons" --file=main.scons %*

:end
