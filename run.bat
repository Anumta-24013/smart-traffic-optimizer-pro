@echo off
echo Starting Smart Traffic Route Optimizer...
echo.

if not exist "build\traffic_optimizer.exe" (
    echo ERROR: Executable not found!
    echo Please run build.bat first.
    pause
    exit /b 1
)

build\traffic_optimizer.exe %*
