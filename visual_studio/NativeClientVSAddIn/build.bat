@echo off
setlocal

:: Set up the Visual Studio environment
call "%VS100COMNTOOLS%vsvars32.bat"
msbuild "%~dp0NativeClientVSAddIn.sln"
python "%~dp0create_package.py"

endlocal
