<#
PowerShell 脚本：自动下载并启动 Qt 在线安装器
使用说明：
  1. 将此脚本保存为 download-and-install-qt.ps1
  2. 在 PowerShell 中运行：
       .\download-and-install-qt.ps1
     或指定下载 URL：
       .\download-and-install-qt.ps1 -InstallerUrl "https://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-<ver>-online.exe"
  3. 脚本会把安装器下载到临时目录并提示是否以管理员权限运行安装器。

注意：
  - 我无法为你远程执行脚本；请在本机运行它。
  - 如果你不确定安装器 URL，脚本会打开 Qt 官方在线安装器页面，供你手动复制链接。
#>

param(
    [string]$InstallerUrl = "",
    [string]$DownloadDir = "$env:TEMP\qt_installer"
)

function Is-Administrator {
    $current = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $current.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if ([string]::IsNullOrWhiteSpace($InstallerUrl)) {
    Write-Host "未指定 Installer URL。"
    Write-Host "如果你已知在线安装器完整 URL，请粘贴并回车；否则输入 'open' 打开官方下载页面以获取 URL，或直接回车取消。"
    $inputUrl = Read-Host "Installer URL 或输入 open"
    if ($inputUrl -eq "open") {
        Start-Process "https://download.qt.io/official_releases/online_installers/"
        Write-Host "已打开浏览器，请在页面选择与你的编译器匹配的安装器并复制URL粘贴回此脚本。"
        $InstallerUrl = Read-Host "粘贴完整的安装器 URL（或回车取消）"
        if ([string]::IsNullOrWhiteSpace($InstallerUrl)) {
            Write-Host "已取消。"
            exit 0
        }
    } elseif (-not [string]::IsNullOrWhiteSpace($inputUrl)) {
        $InstallerUrl = $inputUrl
    } else {
        Write-Host "已取消。"
        exit 0
    }
}

# 创建下载目录
if (-not (Test-Path -Path $DownloadDir)) {
    New-Item -ItemType Directory -Path $DownloadDir -Force | Out-Null
}

$InstallerFile = Join-Path $DownloadDir ([IO.Path]::GetFileName($InstallerUrl))

Write-Host "准备从： $InstallerUrl"
Write-Host "下载到： $InstallerFile"

try {
    Write-Host "开始下载..."
    Invoke-WebRequest -Uri $InstallerUrl -OutFile $InstallerFile -UseBasicParsing -TimeoutSec 600
    Write-Host "下载完成。文件： $InstallerFile"
} catch {
    Write-Error "下载失败： $_"
    exit 2
}

# 执行安装器（需要时提升权限）
$needAdmin = -not (Is-Administrator)
if ($needAdmin) {
    Write-Host "当前不是管理员，会在需要时请求提升权限以运行安装器。"
}

while ($true) {
    $ans = Read-Host "现在运行安装器并以管理员权限安装？(Y/N)"
    if ($ans -match '^[Yy]') {
        try {
            if ($needAdmin) {
                Write-Host "以管理员权限启动安装器..."
                Start-Process -FilePath $InstallerFile -Verb runAs
            } else {
                Write-Host "以当前用户启动安装器..."
                Start-Process -FilePath $InstallerFile
            }
            Write-Host "安装器已启动。请在安装器界面选择 Qt 版本（与 MinGW 匹配）并包含 Qt3D 模块。"
        } catch {
            Write-Error "启动安装器失败：$_"
            exit 3
        }
        break
    } elseif ($ans -match '^[Nn]') {
        Write-Host "已取消启动安装器。你可以稍后运行： $InstallerFile"
        break
    } else {
        Write-Host "请输入 Y 或 N。"
    }
}

# 提示后续步骤
Write-Host "安装完成后请记下 Qt 安装目录（例如 C:\Qt\6.x.x\mingw_64）。"
Write-Host "然后告诉我该路径，我可以为你自动把 QT_BIN_DIR 写入 CMakeLists.txt 或生成自动复制 DLL 的脚本。"
