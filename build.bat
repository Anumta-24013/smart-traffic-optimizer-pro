@echo off
echo ============================================
echo   Smart Traffic Route Optimizer - Build
echo ============================================
echo.

:: Check for g++ compiler
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW or Visual Studio with C++ support.
    echo.
    echo Download MinGW from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo Compiling...
echo.

:: Create build directory
if not exist "build" mkdir build

:: Compile with g++
g++ -std=c++17 -O2 -Wall -o build/traffic_optimizer.exe main.cpp -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo   BUILD SUCCESSFUL!
    echo ============================================
    echo.
    echo Executable: build\traffic_optimizer.exe
    echo.
    echo Run options:
    echo   build\traffic_optimizer.exe           - CLI mode
    echo   build\traffic_optimizer.exe --server  - API server mode
    echo   build\traffic_optimizer.exe --test    - Run tests
    echo.
) else (
    echo.
    echo BUILD FAILED! Check errors above.
)

pause
