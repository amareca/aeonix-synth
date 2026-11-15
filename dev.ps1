# Single entry point for AEONIX development.
#
# Usage:
#   .\dev.ps1 help
#   .\dev.ps1 doctor
#   .\dev.ps1 build [-Configuration Debug] [-Flavour Pro] [-Target aeonix_tests] [-StopAeonix]
#   .\dev.ps1 test [-Tag engine] [-Repeat 5] [-Flavour Pro]
#   .\dev.ps1 verify [-Flavour Pro]
#   .\dev.ps1 ui [-Quick] [-Languages es,en] [-Themes ...] [-Sizes 1180x752] [-OutputDir ...]
#   .\dev.ps1 presets
#   .\dev.ps1 captures [-Clean|-Legacy|-Open]
#
# The .ps1 files stay ASCII: Windows PowerShell 5.1 cannot read UTF-8 without BOM.

$ErrorActionPreference = "Stop"

function Show-Usage
{
    Write-Host ""
    Write-Host "AEONIX - development commands" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  .\dev.ps1 doctor    Checks VS, CMake, JUCE, build and paths."
    Write-Host "  .\dev.ps1 build     Builds DEV Release (or Debug with -Configuration Debug)."
    Write-Host "  .\dev.ps1 test      Runs every test (or -Tag engine)."
    Write-Host "  .\dev.ps1 verify    Configures, builds Release and runs ctest."
    Write-Host "  .\dev.ps1 ui        F11/F12 matrix (48/48) or -Quick (1 combination)."
    Write-Host "  .\dev.ps1 presets   Loads sample presets through the manager."
    Write-Host "  .\dev.ps1 captures  Summarises captures; -Clean, -Legacy or -Open."
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 build -Configuration Debug" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 build -Flavour Pro" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 verify -Pro" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 test -Tag preset -Repeat 5" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 ui -Sizes 1180x752 -NoSnapshot" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 ui -Quick" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 captures -Legacy" -ForegroundColor DarkGray
    Write-Host "  .\dev.ps1 captures -Open" -ForegroundColor DarkGray
    Write-Host ""
}

$all = @($args)

if ($all.Count -eq 0)
{
    Show-Usage
    exit 0
}

$command = [string]$all[0]
$rest = @()

if ($all.Count -gt 1)
{
    $rest = $all[1..($all.Count - 1)]
}

if ($command -in @("help", "-h", "--help", "/?"))
{
    Show-Usage
    exit 0
}

$scripts = @{
    "doctor"  = "doctor.ps1"
    "build"   = "build.ps1"
    "test"    = "test.ps1"
    "verify"  = "verify.ps1"
    "ui"       = "layout-matrix.ps1"
    "presets"  = "presets-smoke.ps1"
    "captures" = "captures.ps1"
}

if (-not $scripts.ContainsKey($command))
{
    Write-Host ("Unknown command: {0}" -f $command) -ForegroundColor Red
    Show-Usage
    exit 1
}

$script = Join-Path $PSScriptRoot ("tools\" + $scripts[$command])

try
{
    & $script @rest
}
catch
{
    Write-Host ""
    Write-Host ("Error: {0}" -f $_.Exception.Message) -ForegroundColor Red
    exit 1
}
