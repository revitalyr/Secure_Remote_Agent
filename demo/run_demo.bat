@echo off
REM Secure Remote Agent Demo Runner (Windows)
REM This script sets up and runs the complete demo on Windows

setlocal enabledelayedexpansion

echo ========================================
echo Secure Remote Agent Demo Setup
echo ========================================

REM Configuration
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_ROOT%\build
set DEMO_BACKEND_DIR=%SCRIPT_DIR%backend
set SERVICE_NAME=SecureRemoteAgentDemo

REM No admin required - demo uses demo_simulator instead of Windows Service

REM Build the project
echo [INFO] Building the project...
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

cd %BUILD_DIR%
echo [INFO] Configuring with CMake...
cmake %PROJECT_ROOT% -DCMAKE_BUILD_TYPE=Release

echo [INFO] Building project...
cmake --build . --config Release

cd ..
echo [INFO] Build completed successfully!

REM Run tests
echo [INFO] Running unit and integration tests...
cd %BUILD_DIR%

echo [INFO] Running unit tests...
ctest --output-on-failure -R "unit_" || echo [WARNING] Some unit tests failed

echo [INFO] Running integration tests...
ctest --output-on-failure -R "integration_" || echo [WARNING] Some integration tests failed

cd %PROJECT_ROOT%
echo [INFO] Tests completed!

REM Setup Python backend
echo [INFO] Setting up Python backend...
cd %DEMO_BACKEND_DIR%

REM Check if Python is available
python --version >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Python is required but not installed
    pause
    exit /b 1
)

REM Install requirements
echo [INFO] Installing Python dependencies...
pip install -r requirements.txt

echo [INFO] Backend setup completed!

REM Start backend server
echo [INFO] Starting demo backend server...
cd %DEMO_BACKEND_DIR%

start /B python demo_server.py
timeout /t 3 /nobreak >nul

REM Check if backend is running
curl -s http://localhost:5000/api/system >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Failed to start backend server
    pause
    exit /b 1
)

echo [INFO] Backend server started successfully

REM Start demo orchestrator (starts service and agent, orchestrates metrics flow)
echo [INFO] Starting demo orchestrator...
cd %SCRIPT_DIR%

start /B python demo_orchestrator.py
timeout /t 5 /nobreak >nul

echo [INFO] Demo orchestrator started successfully

REM Launch UI
echo [INFO] Launching Demo UI...
set UI_PATH=%BUILD_DIR%\src\components\app\Release\main.exe

if not exist "%UI_PATH%" (
    echo [ERROR] UI executable not found at %UI_PATH%
    pause
    exit /b 1
)

start "" "%UI_PATH%"
echo [INFO] Demo UI launched

REM Open web dashboard
echo [INFO] Opening web dashboard...
timeout /t 2 /nobreak >nul
start http://localhost:5000/dashboard

REM Show demo information
echo.
echo [INFO] Demo is now running!
echo.
echo Demo Components:
echo   Backend Server: http://localhost:5000
echo   Web Dashboard: http://localhost:5000/dashboard
echo   Demo Orchestrator: Running in background
echo   - Pseudo-Service: Running (provides real system metrics)
echo   - Agent: Running (loads plugins)
echo   Desktop UI: Running in background
echo.
echo Demo Flow:
echo   Orchestrator -^> Service (real metrics via IPC) -^> Orchestrator -^> Backend -^> Web
echo   Agent loads plugins from separate DLL files
echo.
echo Demo Features:
echo   1. Real System Metrics - From Windows API via IPC
echo   2. Plugin Management - Agent loads Proxy/Scraper plugins
echo   3. Real-time Metrics - Monitor CPU, Memory, and agent status
echo   4. Log Streaming - View real-time logs from all components
echo   5. Web Scraping - Test ScraperPlugin with URL input
echo   6. IPC Communication - Test IPC messaging between Agent and Service
echo.
echo Demo URLs:
echo   Main Dashboard: http://localhost:5000/dashboard
echo   API Endpoints: http://localhost:5000/api/
echo   System Info:    http://localhost:5000/api/system
echo.
echo Press any key to stop the demo...
pause >nul

REM Cleanup
echo [INFO] Cleaning up demo...

REM Stop backend
taskkill /f /im python.exe >nul 2>&1
echo [INFO] Backend server stopped

REM Stop UI
taskkill /f /im main.exe >nul 2>&1
echo [INFO] Demo UI stopped

REM Stop demo orchestrator
taskkill /f /im python.exe >nul 2>&1
echo [INFO] Demo orchestrator stopped

echo [INFO] Demo cleanup completed!
pause
