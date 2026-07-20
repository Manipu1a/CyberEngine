[CmdletBinding()]
param(
    [string]$RepoUrl = "https://github.com/baldurk/renderdoc.git",
    [string]$Branch = "v1.x",
    [ValidateSet("Release", "Development")]
    [string]$Configuration = "Release",
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",
    [string]$PlatformToolset = "v143",
    [switch]$SkipBuild,
    [switch]$Fresh
)

$ErrorActionPreference = "Stop"

$PluginRoot = Split-Path -Parent $PSCommandPath
$SourceDir = Join-Path $PluginRoot "source"
$BinDir = Join-Path $PluginRoot "bin"

function Find-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -property installationPath
        if ($installPath) {
            $candidate = Join-Path $installPath "MSBuild\Current\Bin\MSBuild.exe"
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    $cmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }

    throw "MSBuild was not found. Install Visual Studio Build Tools or Visual Studio 2022 with MSBuild."
}

if ($Fresh -and $SkipBuild) {
    throw "-Fresh cannot be combined with -SkipBuild."
}

if ($Fresh -and (Test-Path $SourceDir)) {
    Remove-Item -LiteralPath $SourceDir -Recurse -Force
}

if (!$SkipBuild) {
    if (!(Test-Path $SourceDir)) {
        git clone --branch $Branch --recursive $RepoUrl $SourceDir
    } else {
        git -C $SourceDir fetch origin $Branch
        git -C $SourceDir checkout $Branch
        git -C $SourceDir pull --ff-only
        git -C $SourceDir submodule update --init --recursive
    }

    $msbuild = Find-MSBuild
    $solution = Join-Path $SourceDir "renderdoc.sln"
    $msbuildArgs = @(
        $solution,
        "/m",
        "/p:Configuration=$Configuration",
        "/p:Platform=$Platform"
    )

    if ($PlatformToolset) {
        $msbuildArgs += "/p:PlatformToolset=$PlatformToolset"
    }

    & $msbuild @msbuildArgs
    if ($LASTEXITCODE -ne 0) {
        throw "RenderDoc build failed."
    }
}

$outDir = Join-Path $SourceDir "$Platform\$Configuration"
if (!(Test-Path $outDir)) {
    throw "RenderDoc output directory was not found: $outDir"
}

New-Item -ItemType Directory -Force -Path $BinDir | Out-Null

$topLevelPatterns = @(
    "*.exe",
    "*.dll",
    "*.pyd",
    "*.zip",
    "*.json",
    "renderdoc_app.h"
)

foreach ($pattern in $topLevelPatterns) {
    Get-ChildItem -LiteralPath $outDir -Filter $pattern -File -ErrorAction SilentlyContinue | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $BinDir -Force
    }
}

$runtimeDirs = @("qtplugins", "pymodules")
foreach ($dirName in $runtimeDirs) {
    $dir = Join-Path $outDir $dirName
    if (Test-Path $dir) {
        Copy-Item -LiteralPath $dir -Destination $BinDir -Recurse -Force
    }
}

$dll = Join-Path $BinDir "renderdoc.dll"
if (!(Test-Path $dll)) {
    throw "renderdoc.dll was not copied. Check RenderDoc build output in: $outDir"
}

$uiExe = Join-Path $BinDir "qrenderdoc.exe"
$legacyUiExe = Join-Path $BinDir "renderdocui.exe"
if (!(Test-Path $uiExe) -and !(Test-Path $legacyUiExe)) {
    Write-Warning "No RenderDoc UI executable was copied. Captures will still save, but CyberEngine cannot auto-open them from the plugin bin directory."
}

Write-Host "RenderDoc binaries copied to: $BinDir"
