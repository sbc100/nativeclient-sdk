@echo off
python.exe "%~dp0install.py" %*
if not defined NO_PAUSE pause
