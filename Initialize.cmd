@echo off
cd /d "%~dp0"
"%~dp0runtime\node.exe" "%~dp0host\setup.mjs" %*
if errorlevel 1 goto end
"%~dp0runtime\node.exe" "%~dp0host\server.mjs"
:end
pause
