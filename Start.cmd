@echo off
cd /d "%~dp0"
"%~dp0runtime\node.exe" "%~dp0host\server.mjs" %*
pause
