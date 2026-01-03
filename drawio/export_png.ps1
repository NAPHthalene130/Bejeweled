param (
    [string]$DrawioPath = "C:\Program Files\draw.io\draw.io.exe",
    [string]$SrcDir = "$PSScriptRoot",
    [string]$DstDir = "$PSScriptRoot\ClientClassPng"
)

if (-not (Test-Path $DstDir)) {
    New-Item -ItemType Directory -Path $DstDir | Out-Null
}

if (-not (Test-Path $DrawioPath)) {
    # Try x86 path
    $DrawioPath = "C:\Program Files (x86)\draw.io\draw.io.exe"
    if (-not (Test-Path $DrawioPath)) {
        # Try Local AppData path
        $DrawioPath = "$env:LOCALAPPDATA\Programs\draw.io\draw.io.exe"
        if (-not (Test-Path $DrawioPath)) {
            Write-Error "Draw.io executable not found. Please install Draw.io Desktop or specify the path."
            exit 1
        }
    }
}

Get-ChildItem -Path $SrcDir -Filter *.drawio | ForEach-Object {
    $outFile = Join-Path $dstDir ($_.BaseName + ".png")
    Write-Host "Exporting $($_.Name) to $outFile"
    try {
        Start-Process -FilePath $DrawioPath -ArgumentList "-x", "-f", "png", "-o", "`"$outFile`"", "`"$($_.FullName)`"" -Wait -NoNewWindow
    }
    catch {
        Write-Error "Failed to export $($_.Name)"
    }
}

Write-Host "Export complete."
