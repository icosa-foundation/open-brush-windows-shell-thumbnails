using SharpShell.Attributes;
using SharpShell.SharpThumbnailHandler;
using System;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Runtime.InteropServices;

namespace Voxels.ShellExtensions {
    [ComVisible(true)]
    [COMServerAssociation(AssociationType.FileExtension, ".tilt")]
    public class ThumbnailHandlerTilt : SharpThumbnailHandler {
        protected override Bitmap GetThumbnailImage(uint width) {
            using (var archive = new ZipArchive(SelectedItemStream, ZipArchiveMode.Read, false)) {
                var entry = archive.GetEntry("thumbnail.png");
                if (entry == null) {
                    foreach (var e in archive.Entries) {
                        if (string.Equals(e.Name, "thumbnail.png", StringComparison.OrdinalIgnoreCase)) {
                            entry = e;
                            break;
                        }
                    }
                }
                if (entry == null)
                    return null;
                using (var entryStream = entry.Open()) {
                    using (var image = (Bitmap)Image.FromStream(entryStream)) {
                        var size = (int)width;
                        if (image.Width == size && image.Height == size) {
                            return new Bitmap(image);
                        }
                        return new Bitmap(image, new Size(size, size));
                    }
                }
            }
        }
    }
}
