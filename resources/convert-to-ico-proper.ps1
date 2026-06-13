Add-Type -AssemblyName System.Drawing

$pngPath = "C:\Users\andyb\Documents\open-brush-windows-shell-thumbnails2\resources\openbrush-source.png"
$icoPath = "C:\Users\andyb\Documents\open-brush-windows-shell-thumbnails2\resources\openbrush-icon.ico"

# Load the PNG
$png = [System.Drawing.Image]::FromFile($pngPath)

# Create bitmaps at standard icon sizes
$sizes = @(16, 32, 48, 256)
$encoder = [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object { $_.MimeType -eq 'image/png' }
$encoderParams = New-Object System.Drawing.Imaging.EncoderParameters(1)
$encoderParams.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter([System.Drawing.Imaging.Encoder]::Quality, 100)

# Create ICO using .NET Icon class with multiple sizes
$memoryStream = New-Object System.IO.MemoryStream
$iconWriter = New-Object System.IO.BinaryWriter($memoryStream)

# Write ICO header
$iconWriter.Write([UInt16]0)  # Reserved
$iconWriter.Write([UInt16]1)  # Type (1 = ICO)
$iconWriter.Write([UInt16]$sizes.Count)  # Number of images

# Prepare image data
$imageDataList = @()
$offset = 6 + (16 * $sizes.Count)

foreach ($size in $sizes) {
    $bitmap = New-Object System.Drawing.Bitmap($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.DrawImage($png, 0, 0, $size, $size)
    $graphics.Dispose()

    $bmpStream = New-Object System.IO.MemoryStream
    $bitmap.Save($bmpStream, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmpData = $bmpStream.ToArray()
    $bmpStream.Dispose()
    $bitmap.Dispose()

    # Write ICO directory entry
    $sizeValue = if ($size -eq 256) { 0 } else { $size }
    $iconWriter.Write([Byte]$sizeValue)  # Width
    $iconWriter.Write([Byte]$sizeValue)  # Height
    $iconWriter.Write([Byte]0)  # Color palette
    $iconWriter.Write([Byte]0)  # Reserved
    $iconWriter.Write([UInt16]1)  # Color planes
    $iconWriter.Write([UInt16]32)  # Bits per pixel
    $iconWriter.Write([UInt32]$bmpData.Length)  # Image data size
    $iconWriter.Write([UInt32]$offset)  # Image data offset

    $imageDataList += $bmpData
    $offset += $bmpData.Length
}

# Write all image data
foreach ($imageData in $imageDataList) {
    $iconWriter.Write($imageData)
}

$iconWriter.Flush()
$icoData = $memoryStream.ToArray()
[System.IO.File]::WriteAllBytes($icoPath, $icoData)

$iconWriter.Close()
$memoryStream.Close()
$png.Dispose()

Write-Host "Multi-resolution ICO file created: $icoPath"
