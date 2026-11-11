# Utilidades compartidas por los scripts de desarrollo de AEONIX.
#
# Los .ps1 se mantienen en ASCII a proposito: Windows PowerShell 5.1 no lee
# UTF-8 sin BOM y los acentos se corrompen. La documentacion si lleva tildes.

$script:RepoRoot = Split-Path -Parent $PSScriptRoot
$script:VsBuildTools = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$script:ConfigBackups = @{}
$script:LibraryBackups = @{}

function Get-RepoRoot
{
    return $script:RepoRoot
}

function Get-VsBuildToolsPath
{
    return $script:VsBuildTools
}

function Find-Executable
{
    param(
        [string]$Name,
        [string[]]$Candidates = @()
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -ne $command)
    {
        return $command.Source
    }

    foreach ($candidate in $Candidates)
    {
        if ($candidate -and (Test-Path -LiteralPath $candidate))
        {
            return $candidate
        }
    }

    return $null
}

function Get-CMakePath
{
    $path = Find-Executable -Name "cmake" -Candidates @(
        (Join-Path $script:VsBuildTools "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"))

    if (-not $path)
    {
        throw "No se encontro CMake. Anade Visual Studio Build Tools o CMake al PATH."
    }

    return $path
}

function Get-CTestPath
{
    $path = Find-Executable -Name "ctest" -Candidates @(
        (Join-Path $script:VsBuildTools "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"))

    if (-not $path)
    {
        throw "No se encontro CTest. Anade Visual Studio Build Tools o CMake al PATH."
    }

    return $path
}

function Get-AeonixBuildDir
{
    param(
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    if ($Flavour -eq "Pro")
    {
        return Join-Path $script:RepoRoot "build-pro"
    }

    return Join-Path $script:RepoRoot "build"
}

function Get-AeonixExePath
{
    param(
        [ValidateSet("Release", "Debug")]
        [string]$Configuration = "Release",

        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    $name = if ($Flavour -eq "Pro") { "AEONIX.exe" } else { "AEONIX-dev.exe" }
    return Join-Path (Get-AeonixBuildDir -Flavour $Flavour) (
        "Aeonix_artefacts\{0}\{1}" -f $Configuration, $name)
}

function Get-AeonixTestsPath
{
    param(
        [ValidateSet("Release", "Debug")]
        [string]$Configuration = "Release",

        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    return Join-Path (Get-AeonixBuildDir -Flavour $Flavour) (
        "{0}\aeonix_tests.exe" -f $Configuration)
}

function Get-AeonixConfigPath
{
    param(
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    $folder = if ($Flavour -eq "Pro") { "Aeonix" } else { "AeonixDev" }
    return Join-Path $env:APPDATA ("{0}\config.json" -f $folder)
}

function Get-AeonixPresetsDir
{
    param(
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    $folder = if ($Flavour -eq "Pro") { "Aeonix" } else { "AeonixDev" }
    return Join-Path ([Environment]::GetFolderPath("MyDocuments")) ("{0}\Presets" -f $folder)
}

function Get-AeonixLibraryPath
{
    param(
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    $folder = if ($Flavour -eq "Pro") { "Aeonix" } else { "AeonixDev" }
    return Join-Path ([Environment]::GetFolderPath("MyDocuments")) ("{0}\library.json" -f $folder)
}

function Get-AeonixSnapshotDir
{
    param([string]$Tag = "")

    $root = Join-Path $env:TEMP "Aeonix\snapshots"

    if ($Tag)
    {
        return Join-Path $root $Tag
    }

    return $root
}

function Get-ReportsDir
{
    $directory = Join-Path $script:RepoRoot "build\reports"

    if (-not (Test-Path -LiteralPath $directory))
    {
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
    }

    return $directory
}

function Write-JsonFile
{
    param(
        [string]$Path,
        $Object
    )

    $directory = Split-Path -Parent $Path

    if ($directory -and -not (Test-Path -LiteralPath $directory))
    {
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
    }

    $json = $Object | ConvertTo-Json -Depth 8
    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $json, $utf8)
}

function Write-Step
{
    param([string]$Text)

    Write-Host ""
    Write-Host ("==> {0}" -f $Text) -ForegroundColor Cyan
}

function Write-Ok
{
    param([string]$Text)
    Write-Host ("  OK  {0}" -f $Text) -ForegroundColor Green
}

function Write-Fail
{
    param([string]$Text)
    Write-Host ("  FALLO  {0}" -f $Text) -ForegroundColor Red
}

function Write-Warn
{
    param([string]$Text)
    Write-Host ("  AVISO  {0}" -f $Text) -ForegroundColor Yellow
}

function Invoke-Checked
{
    param(
        [string]$Exe,
        [string[]]$Arguments,
        [string]$Step
    )

    Write-Step $Step
    & $Exe @Arguments

    if ($LASTEXITCODE -ne 0)
    {
        throw ("{0} fallo con codigo {1}." -f $Step, $LASTEXITCODE)
    }
}

function Get-AeonixProcess
{
    return @(Get-Process -Name "AEONIX", "AEONIX-dev" -ErrorAction SilentlyContinue)
}

function Assert-AeonixNotRunning
{
    $processes = Get-AeonixProcess

    if ($processes.Count -gt 0)
    {
        throw "AEONIX esta en ejecucion; cierra la app antes de compilar (o usa -StopAeonix)."
    }
}

function Stop-AeonixProcess
{
    param([int]$TimeoutSeconds = 5)

    foreach ($process in (Get-AeonixProcess))
    {
        try { [void]$process.CloseMainWindow() } catch { }
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-AeonixProcess).Count -gt 0 -and (Get-Date) -lt $deadline)
    {
        Start-Sleep -Milliseconds 200
    }

    foreach ($process in (Get-AeonixProcess))
    {
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    }
}

function Read-AeonixConfig
{
    $path = Get-AeonixConfigPath

    if (-not (Test-Path -LiteralPath $path))
    {
        return $null
    }

    try
    {
        return (Get-Content -LiteralPath $path -Raw | ConvertFrom-Json)
    }
    catch
    {
        return $null
    }
}

function Set-AeonixConfigJson
{
    param(
        [string]$Json,
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev"
    )

    $path = Get-AeonixConfigPath -Flavour $Flavour
    $directory = Split-Path -Parent $path

    if (-not (Test-Path -LiteralPath $directory))
    {
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
    }

    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($path, $Json, $utf8)
}

function Set-AeonixConfigValues
{
    param([System.Collections.IDictionary]$Values)

    $Json = [ordered]@{}

    foreach ($key in $Values.Keys)
    {
        $Json[$key] = $Values[$key]
    }

    Set-AeonixConfigJson -Json ($Json | ConvertTo-Json -Compress)
}

function New-AeonixBaseConfig
{
    param(
        [string]$Theme = "holo.glass",
        [string]$Language = "es",
        [string]$Preset = "Default",
        [string]$TextSize = ""
    )

    $config = [ordered]@{
        "preset.last"      = $Preset
        "keyboard.octave"  = 0
        "window.valid"     = 1
        "window.x"         = 100
        "window.y"         = 90
        "window.w"         = 1182
        "window.h"         = 754
        "window.maximised" = 0
        "window.chrome"    = 1
        "theme"            = $Theme
        "ui.language"      = $Language
        "polyphony"        = 16
    }

    if ($TextSize)
    {
        $config["ui.textSize"] = $TextSize
    }

    return $config
}

function Backup-AeonixConfig
{
    param([ValidateSet("Dev", "Pro")][string]$Flavour = "Dev")

    $path = Get-AeonixConfigPath -Flavour $Flavour
    $exists = Test-Path -LiteralPath $path

    $script:ConfigBackups[$Flavour] = [pscustomobject]@{
        Exists = $exists
        Bytes  = if ($exists) { [System.IO.File]::ReadAllBytes($path) } else { $null }
    }
}

function Restore-AeonixConfig
{
    param([ValidateSet("Dev", "Pro")][string]$Flavour = "Dev")

    $backup = $script:ConfigBackups[$Flavour]

    if ($null -eq $backup)
    {
        throw ("No hay copia de configuracion para {0}." -f $Flavour)
    }

    $path = Get-AeonixConfigPath -Flavour $Flavour

    if ($backup.Exists)
    {
        $directory = Split-Path -Parent $path

        if (-not (Test-Path -LiteralPath $directory))
        {
            New-Item -ItemType Directory -Path $directory -Force | Out-Null
        }

        [System.IO.File]::WriteAllBytes($path, $backup.Bytes)
    }
    elseif (Test-Path -LiteralPath $path)
    {
        Remove-Item -LiteralPath $path -Force
    }
}

function Backup-AeonixLibrary
{
    param([ValidateSet("Dev", "Pro")][string]$Flavour = "Dev")

    $path = Get-AeonixLibraryPath -Flavour $Flavour
    $exists = Test-Path -LiteralPath $path

    $script:LibraryBackups[$Flavour] = [pscustomobject]@{
        Exists = $exists
        Bytes  = if ($exists) { [System.IO.File]::ReadAllBytes($path) } else { $null }
    }
}

function Restore-AeonixLibrary
{
    param([ValidateSet("Dev", "Pro")][string]$Flavour = "Dev")

    $backup = $script:LibraryBackups[$Flavour]

    if ($null -eq $backup)
    {
        throw ("No hay copia de library.json para {0}." -f $Flavour)
    }

    $path = Get-AeonixLibraryPath -Flavour $Flavour

    if ($backup.Exists)
    {
        $directory = Split-Path -Parent $path

        if (-not (Test-Path -LiteralPath $directory))
        {
            New-Item -ItemType Directory -Path $directory -Force | Out-Null
        }

        [System.IO.File]::WriteAllBytes($path, $backup.Bytes)
    }
    elseif (Test-Path -LiteralPath $path)
    {
        Remove-Item -LiteralPath $path -Force
    }
}

Export-ModuleMember -Function *
