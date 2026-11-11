# Compila AEONIX con el preset de CMake correspondiente.
#
# Uso:
#   .\tools\build.ps1
#   .\tools\build.ps1 -Configuration Debug
#   .\tools\build.ps1 -Target aeonix_tests
#   .\tools\build.ps1 -Flavour Pro
#   .\tools\build.ps1 -Pro
#   .\tools\build.ps1 -StopAeonix

[CmdletBinding()]
param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",

    [ValidateSet("Dev", "Pro")]
    [string]$Flavour = "Dev",

    [switch]$Pro,

    [string]$Target = "",

    [switch]$Configure,

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
$configurationName = $Configuration.ToLowerInvariant()
$buildPreset = if ($Flavour -eq "Pro") { "pro-" + $configurationName } else { $configurationName }
$cache = Join-Path (Get-AeonixBuildDir -Flavour $Flavour) "CMakeCache.txt"

if ($StopAeonix)
{
    Stop-AeonixProcess
}
elseif ((Get-AeonixProcess).Count -gt 0 -and ($Target -eq "" -or $Target -eq "Aeonix" -or $Target -eq "ALL_BUILD"))
{
    Assert-AeonixNotRunning
}

Push-Location $repo

try
{
    if ($Configure -or -not (Test-Path -LiteralPath $cache))
    {
        Invoke-Checked -Exe $cmake -Arguments @("--preset", $configurePreset) -Step ("Configurando CMake ({0})" -f $Flavour)
    }

    $arguments = @("--build", "--preset", $buildPreset)

    if ($Target)
    {
        $arguments += @("--target", $Target)
    }

    Invoke-Checked -Exe $cmake -Arguments $arguments -Step ("Compilando {0} {1}" -f $Flavour, $Configuration)
    Write-Ok ("Build {0} {1} completado." -f $Flavour, $Configuration)
}
finally
{
    Pop-Location
}
