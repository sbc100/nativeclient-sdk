@echo off

setlocal

PATH=%~dp0..\third_party\cygwin\bin;%PATH%

ln -sfn %~dp0..\toolchain\win_x86\nacl /nacl 2>nul
ln -sfn %~dp0..\toolchain\win_x86\nacl64 /nacl64 2>nul

bash nacl-check-package.sh %*
