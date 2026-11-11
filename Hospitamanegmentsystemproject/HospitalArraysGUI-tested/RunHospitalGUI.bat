@echo off
cd /d "%~dp0"
if not exist "HospitalArraysGUI.exe" (
    echo HospitalArraysGUI.exe is missing. Build the project using the README instructions.
    pause
    exit /b 1
)
start "" "%~dp0HospitalArraysGUI.exe"
