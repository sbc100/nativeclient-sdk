@echo off

setlocal

PATH=%~dp0..\third_party\cygwin\bin;%PATH%

bash nacl-check-package.sh %*
