@echo off

REM Relative path of Third Party tools
set THIRD_PARTY=%CD%\..\third_party


REM
REM Check if make exists, if not we need to decompress a local copy
REM
IF NOT EXIST %THIRD_PARTY%\make\bin\make.exe (
	echo Decompressing GNU Make 3.81

	pushd %THIRD_PARTY%
	gmake-3.81.zip.exe
	popd
)


REM Now reissue the make request on the local version of GNU make
%THIRD_PARTY%\make\bin\make.exe -f Makefile.win %*