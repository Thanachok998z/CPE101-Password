@echo off
cd /d "%~dp0"
gcc -std=c11 -O2 main.c -o PasswordGame.exe -mwindows -lgdi32 -luser32
if errorlevel 1 (echo Build failed & pause & exit /b 1)
start "" PasswordGame.exe
