@echo off
setlocal

set BUILD_ERRORLEVEL=
:: Set up the Visual Studio environment
call "%VS100COMNTOOLS%vsvars32.bat"
msbuild "%~dp0NativeClientVSAddIn.sln"
if %ERRORLEVEL% NEQ 0 goto endbuild

call "%VS110COMNTOOLS%vsvars32.bat"
msbuild "%~dp0NativeClientVSAddIn_2012.sln"
if %ERRORLEVEL% NEQ 0 goto endbuild

python "%~dp0create_package.py"

endlocal & set BUILD_ERRORLEVEL=%ERRORLEVEL%

:endbuild
exit /B %BUILD_ERRORLEVEL%
