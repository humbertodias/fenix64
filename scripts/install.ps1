# Install the latest Fenix release into $HOME\fenix and configure the user environment.
# Usage:
#   irm https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.ps1 | iex
[CmdletBinding()]
param(
    [string]$Repo = $(if ($env:FENIX_REPO) { $env:FENIX_REPO } else { "humbertodias/fenix64" }),
    [string]$InstallDir = $(if ($env:FENIX_HOME) { $env:FENIX_HOME } else { (Join-Path $HOME "fenix") }),
    [string]$Version = $(if ($env:FENIX_VERSION) { $env:FENIX_VERSION } else { "latest" }),
    [string]$Linkage = $(if ($env:FENIX_LINKAGE) { $env:FENIX_LINKAGE } else { "static" })
)

$ErrorActionPreference = "Stop"

function Write-Info([string]$Message) {
    Write-Host "  $Message"
}

function Resolve-Tag {
    if ($Version -ne "latest") {
        return $Version
    }

    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest"
    if (-not $release.tag_name) {
        throw "Could not determine latest release tag"
    }
    return $release.tag_name
}

function Find-Payload([string]$ExtractDir) {
    $fxc = Join-Path $ExtractDir "fxc.exe"
    if (Test-Path $fxc) {
        return $ExtractDir
    }
    $nested = Get-ChildItem -Path $ExtractDir -Directory | Select-Object -First 1
    if (-not $nested) {
        throw "Archive did not contain fxc.exe"
    }
    return $nested.FullName
}

Write-Host "Fenix installer" -ForegroundColor White
$tag = Resolve-Tag
$platform = "windows-mingw"
if ($Linkage -notin @("static", "shared")) {
    throw "FENIX_LINKAGE must be static or shared (got: $Linkage)"
}
$asset = "fenix-$platform-$Linkage-$tag.zip"
$url = "https://github.com/$Repo/releases/download/$tag/$asset"

Write-Info "Version : $tag"
Write-Info "Platform: $platform"
Write-Info "Linkage : $Linkage"
Write-Info "Install : $InstallDir"
Write-Host ""

$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("fenix-install-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tmp | Out-Null

try {
    $zipPath = Join-Path $tmp $asset
    Write-Info "Downloading $asset"
    try {
        Invoke-WebRequest -Uri $url -OutFile $zipPath
    } catch {
        $asset = "fenix-$platform-$tag.zip"
        $url = "https://github.com/$Repo/releases/download/$tag/$asset"
        Write-Info "Retrying legacy asset $asset"
        Invoke-WebRequest -Uri $url -OutFile $zipPath
    }

    $extractDir = Join-Path $tmp "extract"
    Expand-Archive -LiteralPath $zipPath -DestinationPath $extractDir -Force
    $src = Find-Payload $extractDir

    if (Test-Path $InstallDir) {
        Remove-Item -Recurse -Force $InstallDir
    }
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Copy-Item -Path (Join-Path $src "*") -Destination $InstallDir -Recurse -Force
}
finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

[Environment]::SetEnvironmentVariable("FENIX_HOME", $InstallDir, "User")

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $userPath) { $userPath = "" }
$parts = @($userPath -split ";" | Where-Object { $_ -and $_ -ne $InstallDir })
$newPath = (@($InstallDir) + $parts) -join ";"
[Environment]::SetEnvironmentVariable("Path", $newPath, "User")

$env:FENIX_HOME = $InstallDir
$env:Path = "$InstallDir;$env:Path"

Write-Host ""
Write-Host "Installed successfully." -ForegroundColor Green
Write-Info "FENIX_HOME=$InstallDir"
Write-Info "Open a new terminal for PATH changes to apply everywhere."
Write-Info "Then try: fxc -h"
