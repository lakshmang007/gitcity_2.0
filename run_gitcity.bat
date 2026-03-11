@echo off
title GitCity - Virtual Earth Explorer
cd /d "%~dp0build"
gitcity.exe
if errorlevel 1 (
    echo.
    echo [ERROR] GitCity crashed or failed to start.
    echo Make sure you have an internet connection.
    pause
)
