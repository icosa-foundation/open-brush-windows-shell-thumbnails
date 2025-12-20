# TiltThumbs

**Windows Shell Extension for Open Brush / Tilt Brush**

TiltThumbs adds beautiful thumbnail previews for `.tilt` sketch files in Windows Explorer. Each thumbnail displays the embedded preview image from your sketch, with the colorful Open Brush logo overlay in the corner.

## Features

- **Automatic Thumbnails** - See previews of your `.tilt` files directly in Windows Explorer
- **Open Brush Branding** - Each thumbnail includes the Open Brush logo overlay
- **Fast & Lightweight** - Native C++ implementation with minimal overhead
- **Easy Installation** - One-click installer for end users
- **Open Source** - MIT licensed, built with modern C++20

## Installation (For Users)

1. Download `TiltThumbs-Installer.exe` from the [releases page](#)
2. Double-click the installer
3. Click through the installation wizard
4. That's it! Your `.tilt` files now show thumbnails in Explorer

**Note:** You may need to restart Windows Explorer to see thumbnails immediately. Press `Ctrl+Shift+Esc`, find "Windows Explorer", right-click and select "Restart".

## 🔄 Upgrading

To upgrade to a newer version:

1. Simply run the new installer - it will automatically unregister the old version and install the new one
2. Restart Windows Explorer (optional but recommended)

**No need to uninstall first!** The installer handles upgrades automatically.

##  Uninstallation

- **From Start Menu**: Start → TiltThumbs → Uninstall TiltThumbs
- **From Control Panel**: Settings → Apps → TiltThumbs → Uninstall

After uninstalling, you may want to clear the thumbnail cache:
1. Open Disk Cleanup (search in Start menu)
2. Select "Thumbnails"
3. Click OK

![cleanup](media/cleanup.png)

## 🔨 Building from Source

### Prerequisites

1. **Visual Studio 2022** with "Desktop development with C++" workload
   - Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - Includes MSVC compiler, Windows SDK, and CMake

2. **NSIS** (optional, for building installer)
   - Download from [nsis.sourceforge.io](https://nsis.sourceforge.io/Download)
   - Only needed if you want to create the installer executable

### Build Steps

1. Open **x64 Native Tools Command Prompt for VS 2022**

2. Navigate to the repository root and configure:
   ```bat
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```

3. Build the DLL:
   ```bat
   cmake --build build --config Release
   ```

4. **(Optional)** Build the installer:
   ```bat
   cmake --build build --config Release --target installer
   ```

### Output Files

- **DLL**: `build\bin\Release\TiltThumbs.dll`
- **Installer**: `installer\TiltThumbs-Installer.exe` (if built with NSIS)
- **Test tool**: `build\bin\Release\comtest.exe`

### Manual Installation (Development)

For testing during development, you can manually register the DLL:

```bat
cd build\bin
install.bat
```

Run as administrator. The script automatically handles upgrades - it will unregister any previous version before registering the new one. To uninstall, run `uninstall.bat`.

## 🧪 Testing

Use the included test tool to verify thumbnail generation:

```bat
cd build\bin\Release
comtest.exe "path\to\your\sketch.tilt" output.bmp
```

This extracts and saves the thumbnail to `output.bmp`.

## 🏗️ Architecture

- **Language**: C++20
- **Build System**: CMake 3.10+
- **Libraries**:
  - [stb_image](https://github.com/nothings/stb) - PNG decoding
  - [stb_image_resize2](https://github.com/nothings/stb) - Image scaling
  - [mio](https://github.com/mandreyel/mio) - Memory-mapped I/O
- **Format**: .tilt files are ZIP archives with a 16-byte header

## 📄 File Format

Open Brush `.tilt` files have this structure:
```
[16-byte .tilt header]
[Standard ZIP archive containing:]
  ├── thumbnail.png     (preview image)
  ├── metadata.json     (sketch metadata)
  └── data.sketch       (stroke data)
```

TiltThumbs extracts `thumbnail.png` and overlays the Open Brush logo.

## 🤝 Contributing

Contributions are welcome! This project is based on the original [SaiThumbs](https://github.com/Wunkolo/SaiThumbs) by Wunkolo, adapted for Open Brush / Tilt Brush files.

## 📝 License

MIT License - See [LICENSE](LICENSE) file for details.

## 🙏 Credits

- Original SaiThumbs project by [Wunkolo](https://github.com/Wunkolo)
- Open Brush logo and branding by the [Open Brush team](https://openbrush.app)
- Built for the [Open Brush](https://openbrush.app) community
