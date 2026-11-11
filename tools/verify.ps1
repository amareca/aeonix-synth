# Verificacion completa: configura, compila Release y ejecuta todos los tests.
#
# Uso:
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
    Invoke-Checked -Exe $cmake -Arguments @("--preset", $configurePreset) -Step ("Configurando CMake ({0})" -f $Flavour)
    Invoke-Checked -Exe $cmake -Arguments @("--build", "--preset", $buildPreset) -Step ("Compilando {0} Release" -f $Flavour)

    & (Join-Path $PSScriptRoot "test.ps1") -Configuration "Release" -Flavour $Flavour

    Write-Host ""
    Write-Host "Verificacion completada correctamente." -ForegroundColor Green
}
finally
{
    Pop-Location
}
