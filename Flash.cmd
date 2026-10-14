@echo off
if "%~1"=="" goto usage
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Flash-Windows.ps1" -Port "%~1"
pause
exit /b
:usage
echo Run Flash.cmd COM_PORT from a terminal, for example Flash.cmd COM5.
pause
