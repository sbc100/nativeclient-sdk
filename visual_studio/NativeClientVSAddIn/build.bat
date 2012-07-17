@echo off

:: Set up the Visual Studio environment
call "%VS100COMNTOOLS%vsvars32.bat"
msbuild "NativeClientVSAddIn.sln"
python create_package.py