@echo off
setlocal enableextensions
cd /d "%~dp0"

echo.
echo ========================================
echo  TiltThumbs Uninstaller
echo ========================================
echo.
echo This will unregister the thumbnail provider
echo for .tilt files from Windows Explorer.
echo.
echo NOTE: This requires administrator rights.
echo.
pause

echo.
echo Uninstalling TiltThumbs.dll...
regsvr32 /u /s Release\TiltThumbs.dll

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo  SUCCESS!
    echo ========================================
    echo.
    echo TiltThumbs has been uninstalled.
    echo.
    echo You may need to restart Explorer or clear
    echo the thumbnail cache to remove cached images.
    echo.
    echo To clear thumbnail cache:
    echo  1. Open Disk Cleanup
    echo  2. Select "Thumbnails"
    echo  3. Click OK
    echo.
) else (
    echo.
    echo ========================================
    echo  UNINSTALLATION FAILED
    echo ========================================
    echo.
    echo Error code: %ERRORLEVEL%
    echo.
    echo Common issues:
    echo  - Not running as Administrator
    echo  - TiltThumbs was not previously installed
    echo.
    echo Please right-click this file and select
    echo "Run as administrator"
    echo.
)

pause