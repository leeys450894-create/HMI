@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Clean-Solution.ps1" -Path "%~dp0"
pause
