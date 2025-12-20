# Building the TiltThumbs Installer

This directory contains the NSIS script for creating a user-friendly Windows installer.

## Prerequisites

1. Download and install [NSIS (Nullsoft Scriptable Install System)](https://nsis.sourceforge.io/Download)
   - Version 3.0 or later recommended
   - The installer is free and open source

## Building the Installer

### Automated Method (Recommended)

1. **Install NSIS** (one-time): https://nsis.sourceforge.io/Download
2. **Regenerate CMake** (one-time):
   ```bat
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```
3. **Build with installer**:
   ```bat
   cmake --build build --config Release --target installer
   ```

The installer will be created as `TiltThumbs-Installer.exe` in this directory.

### Manual Method

1. Build TiltThumbs in Release mode:
   ```bat
   cmake --build build --config Release
   ```

2. Right-click `TiltThumbs.nsi` and select **"Compile NSIS Script"**

   OR run from command line:
   ```bat
   "C:\Program Files (x86)\NSIS\makensis.exe" TiltThumbs.nsi
   ```

### Fully Automatic (Optional)

To build the installer automatically every time you build in Release mode:
1. Edit `CMakeLists.txt`
2. Uncomment lines 149-154
3. Now `cmake --build build --config Release` will also build the installer

## Distributing

The generated `TiltThumbs-Installer.exe` is a standalone file that can be distributed to users. It:

- ✅ Automatically requests admin elevation
- ✅ Installs to Program Files
- ✅ Registers the DLL automatically
- ✅ Creates an uninstaller in Control Panel
- ✅ Shows clear success/error messages
- ✅ No technical knowledge required from users

## User Experience

**Installation:**
1. User double-clicks `TiltThumbs-Installer.exe`
2. Windows UAC prompts for admin approval
3. Standard installer UI guides them through
4. Done! Thumbnails appear in Explorer

**Uninstallation:**
- From Start Menu → TiltThumbs → Uninstall
- OR from Control Panel → Programs and Features → TiltThumbs → Uninstall
