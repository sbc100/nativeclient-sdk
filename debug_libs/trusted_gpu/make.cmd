@echo off

set TMP_PATH=%PATH%

call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"

REM Relative path of CygWin
set CYGWIN=%~dp0%..\..\third_party\cygwin\bin

PATH=%CYGWIN%;%PATH%

make.exe %*

PATH=%TMP_PATH%