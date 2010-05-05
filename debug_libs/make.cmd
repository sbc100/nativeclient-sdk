@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat"

REM Relative path of CygWin
set CYGWIN=%~dp0%..\third_party\cygwin\bin

PATH=%CYGWIN%;%PATH%

make.exe %*
