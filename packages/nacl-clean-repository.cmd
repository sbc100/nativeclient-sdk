@echo off

setlocal

PATH=%~dp0..\third_party\cygwin\bin;%PATH%

bash nacl-clean-repository.sh %*
