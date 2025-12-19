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

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (ensures the MSVC toolchain, Windows SDK, and CMake integration are available).
2. Open the **x64 Native Tools Command Prompt for VS** (or the matching architecture you want to build).
3. From the repository root, generate build files and build the DLL:
   ```bat
   cmake -S . -B build -G "NMake Makefiles"
   cmake --build build --config Release
   ```
4. The compiled `SaiThumbs.dll` and helper scripts will be under `build\\bin\\`.
5. To register the shell extension for `.tilt` files, run `install.bat` from the output directory in an elevated prompt. To remove it later, run `uninstall.bat`.
