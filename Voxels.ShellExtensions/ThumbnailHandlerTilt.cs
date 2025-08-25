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
            var stream = SelectedItemStream;
            if (stream.CanSeek) {
                stream.Position = 0;
            }
            // .tilt files begin with a 16-byte "tilT" header before the ZIP payload.
            if (stream.CanSeek && stream.Length >= 16) {
                var header = new byte[4];
                stream.Read(header, 0, 4);
                if (header[0] == 't' && header[1] == 'i' && header[2] == 'l' && header[3] == 'T') {
                    stream.Position = 16; // skip custom header
                }
                else {
                    stream.Position = 0; // not a tilt header, rewind
                }
            }
            using (var archive = new ZipArchive(stream, ZipArchiveMode.Read, false)) {
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
