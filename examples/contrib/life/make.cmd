@echo off

set TMP_PATH=%PATH%

REM Relative path of CygWin
set CYGWIN=%~dp0%..\..\..\third_party\cygwin\bin

PATH=%CYGWIN%;%PATH%

make.exe %*

PATH=%TMP_PATH%
