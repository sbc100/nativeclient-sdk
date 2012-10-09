:: This script will build, unpack, and install the add-in onto this machine.  The installation requires admin privledge.
@echo off
setlocal
set OUT_DIR=%~dp0..\..\out\vs_addin
set ZIP_BASE=vs_addin

if exist "%OUT_DIR%\%ZIP_BASE%" (
  del /q /s /f "%OUT_DIR%\%ZIP_BASE%\*.*"
  rmdir /s /q "%OUT_DIR%\%ZIP_BASE%"
)

python.exe -c "import tarfile; zztop=tarfile.open(r'%OUT_DIR%\%ZIP_BASE%.tgz'); zztop.extractall(r'%OUT_DIR%'); zztop.close()"

:: Pass flags to bypass the install questions. Also pipe a key stroke 'return' to pass the 'Press any key to continue' in install.bat
set NO_PAUSE=1
call "%OUT_DIR%\%ZIP_BASE%\install.bat" --force --ppapi
exit /B %ERRORLEVEL%

endlocal
