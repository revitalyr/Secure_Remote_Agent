@echo off
REM Build MSI installer for Secure Remote Agent

setlocal enabledelayedexpansion

echo Building Secure Remote Agent MSI Installer...

REM Check if WiX Toolset is installed
where candle >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: WiX Toolset (candle.exe) not found in PATH
    echo Please install WiX Toolset from https://wixtoolset.org/
    exit /b 1
)

where light >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: WiX Toolset (light.exe) not found in PATH
    echo Please install WiX Toolset from https://wixtoolset.org/
    exit /b 1
)

REM Check if build directory exists
if not exist "build\Release" (
    echo ERROR: build\Release directory not found
    echo Please build the project first using build.bat
    exit /b 1
)

REM Check if required files exist
if not exist "build\Release\main.exe" (
    echo ERROR: main.exe not found in build\Release
    exit /b 1
)

if not exist "build\Release\service_main.exe" (
    echo ERROR: service_main.exe not found in build\Release
    exit /b 1
)

REM Create output directory
if not exist "installer\output" mkdir "installer\output"

REM Generate GUID for UpgradeCode (if not set)
if not defined UPGRADE_CODE (
    echo Generating new UpgradeCode GUID...
    for /f "tokens=*" %%i in ('powershell -Command "[guid]::NewGuid().ToString()"') do set UPGRADE_CODE=%%i
    echo UpgradeCode: !UPGRADE_CODE!
)

REM Replace GUID placeholder in WXS file
powershell -Command "(Get-Content installer\installer.wxs) -replace 'GUID-GOES-HERE', '!UPGRADE_CODE!' | Set-Content installer\installer_temp.wxs"

REM Compile WXS to WIXOBJ
echo Compiling installer.wxs...
candle installer\installer_temp.wxs -out installer\installer.wixobj -ext WixUIExtension
if %errorlevel% neq 0 (
    echo ERROR: candle failed
    exit /b 1
)

REM Link WIXOBJ to MSI
echo Linking installer.wixobj...
light installer\installer.wixobj -out installer\output\SecureRemoteAgent.msi -ext WixUIExtension
if %errorlevel% neq 0 (
    echo ERROR: light failed
    exit /b 1
)

REM Clean up temporary files
del installer\installer_temp.wxs
del installer\installer.wixobj

echo.
echo MSI installer created successfully: installer\output\SecureRemoteAgent.msi
echo.
echo UpgradeCode: !UPGRADE_CODE!

endlocal
