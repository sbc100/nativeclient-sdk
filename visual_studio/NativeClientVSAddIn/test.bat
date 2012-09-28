@echo off

:: Locations
set out_directory=..\..\out\vs_addin\Test
set mstest_file=Results.trx
set test_assembly=..\..\out\vs_addin\Debug\UnitTests.dll

:: Set up the Visual Studio environment
call "%VS100COMNTOOLS%vsvars32.bat"

:: Make the output directory and clean up existing mstest result file
mkdir %out_directory%
if exist %out_directory%\%mstest_file% del %out_directory%\%mstest_file%

:: Run MSTest
mstest /testcontainer:%test_assembly% /testsettings:Local.testsettings /resultsfile:%out_directory%\%mstest_file%

:: Parse the result with python script, return python's exit status
python check_test_results.py %out_directory%\%mstest_file%
exit /B %ERRORLEVEL%
