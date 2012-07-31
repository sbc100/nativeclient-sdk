:: This script will build, unpack, and install the add-in onto this machine.  The installation requires admin privledge.
@echo off
setlocal
set OUT_DIR=%~dp0..\..\out\NativeClientVSAddIn\
set ZIP_BASE=NativeClientVSAddIn

if exist "%OUT_DIR%%ZIP_BASE%" (
  del /q /s /f "%OUT_DIR%%ZIP_BASE%\*.*"
  rmdir /s /q "%OUT_DIR%%ZIP_BASE%"
)

call %~dp0build.bat

python.exe -c "import zipfile; zztop=zipfile.ZipFile(r'%OUT_DIR%%ZIP_BASE%.zip'); zztop.extractall(r'%OUT_DIR%%ZIP_BASE%'); zztop.close()"

:: Pass flags to bypass the install questions. Also pipe a key stroke 'return' to pass the 'Press any key to continue' in install.bat
echo. | call %OUT_DIR%%ZIP_BASE%\install.bat --force --ppapi

endlocal
pause