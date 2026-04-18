@echo off
REM Build script for Secure Remote Agent

echo ========================================
echo Building Secure Remote Agent
echo ========================================

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Configure
echo Configuring with CMake...
cmake --preset default
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build core components
echo Building core library...
cmake --build build --target core --config Release
if %errorlevel% neq 0 (
    echo Core build failed!
    pause
    exit /b 1
)

REM Build service
echo Building Windows Service...
cmake --build build --target service_main --config Release
if %errorlevel% neq 0 (
    echo Service build failed!
    pause
    exit /b 1
)

REM Build UI application
echo Building Desktop UI...
cmake --build build --target main --config Release
if %errorlevel% neq 0 (
    echo UI build failed!
    pause
    exit /b 1
)

REM Build plugins
echo Building Proxy Plugin...
cmake --build build --target proxy_plugin --config Release
if %errorlevel% neq 0 (
    echo Proxy plugin build failed!
    pause
    exit /b 1
)

echo Building Scraper Plugin...
cmake --build build --target scraper_plugin --config Release
if %errorlevel% neq 0 (
    echo Scraper plugin build failed!
    pause
    exit /b 1
)

echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Built components:
echo - Service: build\service\Release\service_main.exe
echo - UI: build\app\Release\main.exe
echo - Proxy Plugin: build\plugins\proxy_plugin.dll
echo - Scraper Plugin: build\plugins\scraper_plugin.dll
echo.
echo To run the demo:
echo 1. Start the backend: cd demo\backend && python demo_server.py
echo 2. Install service: sc create "SecureRemoteAgent" binPath="%cd%\build\service\Release\service_main.exe"
echo 3. Start service: sc start "SecureRemoteAgent"
echo 4. Launch UI: build\app\Release\main.exe
echo.
pause
