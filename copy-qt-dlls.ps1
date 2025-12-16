param(
    [string]$BuildDir,
    [string]$QtBin
)

# Try to read QT_BIN_DIR from CMakeLists.txt if not provided
if (-not $QtBin) {
    $cmake = Join-Path (Get-Location).Path 'CMakeLists.txt'
    if (Test-Path $cmake) {
        $c = Get-Content -Raw -Path $cmake
        if ($c -match 'set\s*\(\s*QT_BIN_DIR\s+"([^"]+)"\s*\)') { $QtBin = $matches[1] }
    }
}

if (-not $QtBin) {
    Write-Error "无法确定 Qt bin 目录。请通过 -QtBin 参数指定（例如 C:/Qt/6.10.1/mingw_64/bin）。"
    exit 1
}

if (-not (Test-Path $QtBin)) {
    Write-Error "指定的 Qt bin 路径不存在： $QtBin"
    exit 2
}

if (-not $BuildDir) {
    $BuildDir = Read-Host "请输入 build 输出目录（例如 D:\\Bejeweled\\build\\Debug）"
}

if (-not (Test-Path $BuildDir)) {
    Write-Error "未找到指定的 build 输出目录： $BuildDir"
    exit 3
}

$DLLs = @(
    'Qt6Widgets.dll',
    'Qt6Network.dll',
    'Qt6Core.dll',
    'Qt6Gui.dll',
    'Qt63DCore.dll',
    'Qt63DRender.dll',
    'Qt63DExtras.dll',
    'Qt63DAnimation.dll',
    'Qt63DInput.dll',
    'Qt63DLogic.dll',
    'Qt63DQuick.dll',
    'Qt63DQuickAnimation.dll',
    'Qt63DQuickExtras.dll',
    'Qt63DQuickInput.dll',
    'Qt63DQuickLogic.dll',
    'Qt63DQuickRender.dll',
    'Qt63DQuickScene2D.dll',
    'Qt63DQuickScene3D.dll',
    'Qt6OpenGL.dll',
    'Qt6OpenGLWidgets.dll',
    'Qt6Quick.dll',
    'Qt6Quick3D.dll',
    'Qt6Quick3DAssetImport.dll',
    'Qt6Quick3DAssetUtils.dll',
    'Qt6Quick3DEffects.dll',
    'Qt6Quick3DGlslParser.dll',
    'Qt6Quick3DHelpers.dll',
    'Qt6Quick3DHelpersImpl.dll',
    'Qt6Quick3DIblBaker.dll',
    'Qt6Quick3DParticleEffects.dll',
    'Qt6Quick3DParticles.dll',
    'Qt6Quick3DRuntimeRender.dll',
    'Qt6Quick3DSpatialAudio.dll',
    'Qt6Quick3DUtils.dll',
    'Qt6Quick3DXr.dll',
    'Qt6QuickWidgets.dll',
    'Qt6Qml.dll',
    'Qt6QmlCore.dll',
    'Qt6QmlModels.dll',
    'Qt6QmlWorkerScript.dll',
    'Qt6QmlNetwork.dll',
    'Qt6QmlXmlListModel.dll',
    'Qt6QuickControls2.dll',
    'Qt6QuickLayouts.dll',
    'Qt6QuickShapes.dll',
    'Qt6QuickTemplates2.dll',
    'Qt6Gui.dll',
    'Qt6PrintSupport.dll',
    'Qt6Sql.dll',
    'Qt6Svg.dll',
    'Qt6SvgWidgets.dll',
    'Qt6Test.dll',
    'Qt6UiTools.dll',
    'Qt6Xml.dll',
    'libgcc_s_seh-1.dll',
    'libstdc++-6.dll',
    'libwinpthread-1.dll',
    'd3dcompiler_47.dll',
    'opengl32sw.dll'
)

Write-Host "使用 Qt bin: $QtBin"
Write-Host "目标 build 目录: $BuildDir"

foreach ($dll in $DLLs) {
    $src = Join-Path $QtBin $dll
    $dst = Join-Path $BuildDir $dll
    if (Test-Path $src) {
        Copy-Item -Path $src -Destination $dst -Force
        Write-Host "已复制: $dll"
    } else {
        Write-Warning "未找到: $dll 在 $QtBin"
    }
}

Write-Host "全部操作完成。" 
