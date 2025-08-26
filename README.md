# Open Brush Windows Explorer Thumbnails

This project provides a Windows Explorer thumbnail handler for Open Brush `.tilt` files.
A `.tilt` file begins with a 16-byte `tilT` header followed by a ZIP archive that contains a
`thumbnail.png`. The handler extracts this image and returns it as the preview so Explorer shows a
miniature of the sketch.

The implementation is based on SharpShell and targets the .NET Framework 4.8. The voxel
format handlers from the original template have been removed.

## Installing

Each GitHub Actions build produces a `OpenBrushShellExtensionsSetup.exe` artifact. Run the
executable to register the shell extension, or run it with `/uninstall` to remove it.
