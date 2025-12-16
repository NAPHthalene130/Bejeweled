<#
detect-and-update-qt.ps1
自动检测本机 Qt 安装（查找 mingw_64）并更新项目的 CMakeLists.txt 中的 QT_BIN_DIR。
使用方法：在项目根（包含 CMakeLists.txt）运行：
  .\detect-and-update-qt.ps1

脚本步骤：
 - 在常见位置查找 Qt 安装 (C:\Qt, C:\Program Files\Qt, %USERPROFILE%\Qt 等)
 - 在每个版本目录查找 mingw_64/bin 下是否存在 Qt6Core.dll 或 qmake.exe
 - 选择最新版本（按路径排序）并构造 bin 路径
 - 备份 CMakeLists.txt 为 CMakeLists.txt.bak.timestamp
 - 将 or 添加 set(QT_BIN_DIR "...") 行到 CMakeLists.txt（替换已存在的）
 - 显示变更摘要并退出

请在运行前关闭任何可能锁定 CMakeLists.txt 的编辑器（可选）。
#>

param(
    [string]$ProjectDir = (Get-Location).Path,
    [switch]$AutoAccept
)

function Find-QtRoots {
    $candidates = @()
    $common = @(
        "C:\\Qt",
        "C:\\Program Files\\Qt",
        "$env:USERPROFILE\\Qt",
        "$env:LOCALAPPDATA\\Programs\\Qt"
    )
    foreach ($p in $common) {
        if (Test-Path $p) { $candidates += Get-ChildItem -Path $p -Directory -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName } }
    }
    # also search one level deeper for 6.x.x style
    foreach ($p in $common) {
        if (Test-Path $p) {
            $sub = Get-ChildItem -Path $p -Directory -Recurse -Depth 2 -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName }
            $candidates += $sub
        }
    }
    $candidates = $candidates | Sort-Object -Unique
    return $candidates
}

function Has-Mingw64Bin($root) {
    $bing = Join-Path $root "mingw_64\bin"
    if (Test-Path $bing) {
        # check for evidence of Qt
        if (Test-Path (Join-Path $bing 'Qt6Core.dll') -PathType Leaf -ErrorAction SilentlyContinue -or Test-Path (Join-Path $bing 'qmake.exe') -PathType Leaf -ErrorAction SilentlyContinue) {
            return $bing
        }
    }
    return $null
}

Write-Host "搜索常见 Qt 安装位置（这可能需要几秒）..."
$roots = Find-QtRoots
$found = @()
foreach ($r in $roots) {
    $b = Has-Mingw64Bin $r
    if ($b) { $found += [PSCustomObject]@{Root=$r; Bin=$b} }
}

if ($found.Count -eq 0) {
    Write-Warning "未在常见路径下找到 mingw_64/bin。请手动提供 Qt 安装路径（例如 C:\Qt\\6.10.1\\mingw_64）。"
    $manual = Read-Host "输入 Qt mingw_64 根目录路径，或回车取消"
    if ([string]::IsNullOrWhiteSpace($manual)) { Write-Host "已取消。"; exit 1 }
    $bing = Join-Path $manual "bin"
    if (-not (Test-Path $bing)) { Write-Error "指定路径下未找到 bin 目录： $bing"; exit 2 }
    $selectedBin = $bing
} else {
    # choose the last (usually the newest) by sorting Root descending
    $sel = $found | Sort-Object Root -Descending | Select-Object -First 1
    $selectedBin = $sel.Bin
    Write-Host "发现 Qt 安装： $($sel.Root) -> bin: $selectedBin"
    if (-not $AutoAccept) {
        $ok = Read-Host "是否使用该路径并更新 CMakeLists.txt ? (Y/N)"
        if ($ok -notmatch '^[Yy]') { Write-Host "已取消。如需手动指定路径请重新运行脚本并输入路径。"; exit 0 }
    }
}

# locate CMakeLists
$cmakeFile = Join-Path $ProjectDir 'CMakeLists.txt'
if (-not (Test-Path $cmakeFile)) { Write-Error "未找到 CMakeLists.txt 于: $cmakeFile"; exit 3 }

# backup
$ts = Get-Date -Format yyyyMMddHHmmss
$bak = "$cmakeFile.bak.$ts"
Copy-Item -Path $cmakeFile -Destination $bak -Force
Write-Host "已备份： $bak"

# prepare replacement text (use forward slashes for CMake)
$binPathForCMake = $selectedBin -replace '\\','/'
$replacementLine = '    set(QT_BIN_DIR "' + $binPathForCMake + '")'

# read, replace or add
$content = Get-Content -Raw -Path $cmakeFile
if ($content -match 'set\(QT_BIN_DIR\s+"[^"]*"\)') {
    $new = [regex]::Replace($content, 'set\(QT_BIN_DIR\s+"[^"]*"\)', [regex]::Escape($replacementLine))
} else {
    # add before WIN32 block or at end
    if ($content -match '(?ms)# 自动复制Qt DLL到输出目录（仅Windows，需本地Qt bin目录）') {
        $new = $content -replace '(?ms)(# 自动复制Qt DLL到输出目录（仅Windows，需本地Qt bin目录）)', "`$1\n$replacementLine\n"
    } else {
        $new = $content + "`n$replacementLine`n"
    }
}

# write back
Set-Content -Path $cmakeFile -Value $new -Encoding UTF8
Write-Host "已更新 $cmakeFile，QT_BIN_DIR = $binPathForCMake"
Write-Host "请重新运行 CMake 配置并构建："
Write-Host "  cmake -S . -B build -G \"MinGW Makefiles\" -DBUILD_SHARED_LIB=ON"
Write-Host "  cmake --build build --config Debug -j 8"

Write-Host "完成。若需我生成自动复制 DLL 的脚本或直接复制 DLL，请把 build 输出目录路径告诉我（例如 D:\\Bejeweled\\build\\Debug）。"
