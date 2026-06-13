@echo off
setlocal enableextensions
cd /d "%~dp0"

echo.
echo ========================================
echo  TiltThumbs Installer
echo ========================================
echo.
echo This will register the thumbnail provider
echo for .tilt files in Windows Explorer.
echo.
echo NOTE: This requires administrator rights.
echo.
pause

echo.
echo Installing TiltThumbs.dll...
echo (Unregistering any previous version first...)
regsvr32 /u /s Release\TiltThumbs.dll 2>nul
echo Registering TiltThumbs.dll...
regsvr32 /s Release\TiltThumbs.dll

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo  SUCCESS!
    echo ========================================
    echo.
    echo TiltThumbs has been installed.
    echo .tilt files will now show thumbnails in Explorer.
    echo.
    echo You may need to restart Explorer or refresh
    echo the folder to see thumbnails.
    echo.
) else (
    echo.
    echo ========================================
    echo  INSTALLATION FAILED
    echo ========================================
    echo.
    echo Error code: %ERRORLEVEL%
    echo.
    echo Common issues:
    echo  - Not running as Administrator
    echo  - DLL file is missing or blocked
    echo.
    echo Please right-click this file and select
    echo "Run as administrator"
    echo.
)

pause