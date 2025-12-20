Add-Type -AssemblyName System.Drawing

$pngPath = "C:\Users\andyb\Documents\open-brush-windows-shell-thumbnails2\resources\overlay-icon.png"
$icoPath = "C:\Users\andyb\Documents\open-brush-windows-shell-thumbnails2\resources\openbrush-icon.ico"

$png = [System.Drawing.Image]::FromFile($pngPath)
$bitmap = New-Object System.Drawing.Bitmap $png
$icon = [System.Drawing.Icon]::FromHandle($bitmap.GetHicon())

$fileStream = New-Object System.IO.FileStream($icoPath, [System.IO.FileMode]::Create)
$icon.Save($fileStream)
$fileStream.Close()

$png.Dispose()
$bitmap.Dispose()

Write-Host "ICO file created: $icoPath"
