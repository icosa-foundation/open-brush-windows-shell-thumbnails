# SaiThumbs [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/Wunkolo/SaiThumbs/master/LICENSE)

SaiThumbs is a Windows Shell extension that will allow image thumbnails for `.tilt` sketch files produced by Tilt Brush-compatible tools. Explorer will display the `thumbnail.png` contained in each archive.

### [Download the latest release here!](https://wunkolo.itch.io/saithumbs)

![logo](media/logo.gif)

![sample](media/scroll.gif)

> Featuring art by [@vintagefoods_​](https://x.com/vintagefoods_)([vf.media](https://vf.media/))!

![install](media/install.gif)

---

Between SaiThumbs installations and updates you ​may​ still see cached thumbnails for `.sai`/`.sai2` files in explorer even after uninstalling or updating to a new SaiThumbs.dll. You can clear your thumbnail cache by running "Disk Cleanup" and selecting "Thumbnails" before cleaning.

![cleanup](media/cleanup.png)

## Building (Windows / Visual Studio)

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload. This pulls in MSVC, the Windows SDK (for `rc.exe`/`mt.exe`), and CMake integration.
2. Open the **x64 Native Tools Command Prompt for VS** (or the architecture you need). This sets up environment variables so CMake can find the Windows SDK tools.
3. From the repository root, generate build files with the Visual Studio generator and build the DLL:
   ```bat
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```
   (You can also use `"NMake Makefiles"` or `"Ninja"` generators if you prefer, but make sure the Windows SDK is installed so `rc.exe`/`mt.exe` are available; missing SDK tools cause the `CMAKE_MT-NOTFOUND`/`rc` errors shown above.)
4. The compiled `SaiThumbs.dll` and helper scripts will be under `build\\bin\\` (or `<build>\\bin\\Release` depending on generator).
5. To register the shell extension for `.tilt` files, run `install.bat` from the output directory in an elevated prompt. To remove it later, run `uninstall.bat`.
