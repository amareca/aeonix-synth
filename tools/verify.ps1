# Full verification: configure, build Release and run every test.
#
# Usage:
#   .\tools\verify.ps1
#   .\tools\verify.ps1 -Pro
#   .\tools\verify.ps1 -Flavour Pro
#   .\tools\verify.ps1 -StopAeonix

[CmdletBinding()]
param(
    [ValidateSet("Dev", "Pro")]
    [string]$Flavour = "Dev",

    [switch]$Pro,

    [switch]$StopAeonix
)

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force

if ($Pro)
{
    $Flavour = "Pro"
}

$repo = Get-RepoRoot
$cmake = Get-CMakePath
$configurePreset = if ($Flavour -eq "Pro") { "pro" } else { "dev" }
$buildPreset = if ($Flavour -eq "Pro") { "pro-release" } else { "release" }

if ($StopAeonix)
{
    Stop-AeonixProcess
}
else
{
    Assert-AeonixNotRunning
}

Push-Location $repo

try
{
    Invoke-Checked -Exe $cmake -Arguments @("--preset", $configurePreset) -Step ("Configuring CMake ({0})" -f $Flavour)
    Invoke-Checked -Exe $cmake -Arguments @("--build", "--preset", $buildPreset) -Step ("Building {0} Release" -f $Flavour)

    & (Join-Path $PSScriptRoot "test.ps1") -Configuration "Release" -Flavour $Flavour

    Write-Host ""
    Write-Host "Verification finished successfully." -ForegroundColor Green
}
finally
{
    Pop-Location
}
