# Alphabet Language Installer for Windows
# Usage: irm https://raw.githubusercontent.com/fraol163/alphabet/main/install.ps1 | iex

$ErrorActionPreference = "Stop"

$REPO = "fraol163/alphabet"
$INSTALL_DIR = if ($env:ALPHABET_DIR) { $env:ALPHABET_DIR } else { "$env:USERPROFILE\alphabet" }

function Write-Info    { param($msg) Write-Host "`u{25B8} $msg" -ForegroundColor Cyan }
function Write-Ok      { param($msg) Write-Host "`u{2713} $msg" -ForegroundColor Green }
function Write-Fail    { param($msg) Write-Host "`u{2717} $msg" -ForegroundColor Red; exit 1 }

# ── Uninstall ──
if ($args.Count -gt 0 -and $args[0] -eq "--uninstall") {
    Remove-Item -Path "$INSTALL_DIR\alphabet.exe" -Force -ErrorAction SilentlyContinue
    Remove-Item -Path "$INSTALL_DIR\alphabet.bat" -Force -ErrorAction SilentlyContinue
    Write-Ok "Uninstalled alphabet from $INSTALL_DIR"
    exit 0
}

Write-Host ""
Write-Host "   ╔══════════════════════════════╗" -ForegroundColor Cyan
Write-Host "   ║   Alphabet Language Installer ║" -ForegroundColor Cyan
Write-Host "   ╚══════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# ── Check dependencies ──
if (-not (Get-Command curl.exe -ErrorAction SilentlyContinue)) {
    Write-Fail "curl.exe is required. It ships with Windows 10 1803+. Please update Windows."
}

# ── Detect arch ──
$arch = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "arm64" } else { "amd64" }
Write-Info "Detected: windows-$arch"

# ── Current version ──
$current = "none"
if (Get-Command alphabet -ErrorAction SilentlyContinue) {
    try {
        $verStr = & alphabet --version 2>&1 | Select-Object -First 1
        if ($verStr -match '(\d+\.\d+\.\d+)') { $current = $Matches[1] }
    } catch {}
    Write-Info "Found existing Alphabet v$current"
}

# ── Fetch latest version ──
Write-Info "Fetching latest version..."
try {
    $releaseJson = curl.exe -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>$null
    $releaseObj = $releaseJson | ConvertFrom-Json
    $latest = $releaseObj.tag_name -replace '^v', ''
} catch {
    Write-Fail "Could not fetch latest version from GitHub"
}

if (-not $latest) { Write-Fail "Could not parse latest version" }
Write-Info "Latest version: v$latest"

if ($current -eq $latest) {
    Write-Ok "Already up to date!"
    exit 0
}

# ── Download ──
$assetName = "alphabet-windows-$arch.exe"
$downloadUrl = "https://github.com/$REPO/releases/download/v$latest/$assetName"

if (-not (Test-Path $INSTALL_DIR)) {
    New-Item -ItemType Directory -Path $INSTALL_DIR -Force | Out-Null
}

$tmpFile = "$INSTALL_DIR\alphabet.exe.tmp"
Write-Info "Downloading $downloadUrl..."

try {
    curl.exe -fsSL --retry 3 --connect-timeout 10 -o $tmpFile $downloadUrl 2>$null
} catch {
    Remove-Item -Path $tmpFile -Force -ErrorAction SilentlyContinue
    Write-Fail "Download failed: $_"
}

if (-not (Test-Path $tmpFile)) {
    Write-Fail "Download failed: file not found"
}

# ── Install ──
Write-Info "Installing..."
$target = "$INSTALL_DIR\alphabet.exe"

try {
    # Replace existing binary (works even if running from same path)
    if (Test-Path $target) {
        Remove-Item -Path $target -Force -ErrorAction Stop
    }
    Move-Item -Path $tmpFile -Destination $target -Force
} catch {
    # If locked, schedule rename on reboot
    Write-Info "Binary in use, scheduling update on reboot..."
    $source = $tmpFile
    $dest = $target
    Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public class MoveEx {
    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern bool MoveFileEx(string lpExistingFileName, string lpNewFileName, int dwFlags);
}
"@
    [MoveEx]::MoveFileEx($source, $dest, 0x1)  # MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT = 0x1
}

# ── Create wrapper bat for cmd.exe ──
$batWrapper = @"
@echo off
"$INSTALL_DIR\alphabet.exe" %*
"@
Set-Content -Path "$INSTALL_DIR\alphabet.bat" -Value $batWrapper -Encoding ASCII

# ── Add to PATH ──
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -notlike "*$INSTALL_DIR*") {
    Write-Info "Adding $INSTALL_DIR to PATH..."
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$INSTALL_DIR", "User")
    $env:Path = "$env:Path;$INSTALL_DIR"
    Write-Info "PATH updated (restart terminal to take effect)"
}

# ── Done ──
Write-Host ""
Write-Ok "Alphabet v$latest installed successfully!"
Write-Host ""
Write-Info "Run 'alphabet --help' to get started."
Write-Info "If 'alphabet' is not found, restart your terminal or run:"
Write-Host "  $INSTALL_DIR\alphabet.exe --help" -ForegroundColor DarkGray
Write-Host ""
