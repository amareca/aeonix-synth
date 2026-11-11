# Comprueba que el entorno de desarrollo esta listo.
#
# Uso:
#   .\tools\doctor.ps1

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force

$repo = Get-RepoRoot
$checks = New-Object System.Collections.Generic.List[object]

function Add-Check
{
    param(
        [string]$Name,
        [string]$Status,
        [string]$Detail
    )

    $checks.Add([pscustomobject]@{
        Comprobacion = $Name
        Estado       = $Status
        Detalle      = $Detail
    })
}

Write-Step "Entorno de desarrollo de AEONIX"

Add-Check "Windows" "OK" ([System.Environment]::OSVersion.VersionString)
Add-Check "PowerShell" "OK" ($PSVersionTable.PSVersion.ToString())

$vsRoot = Get-VsBuildToolsPath

$cmake = Find-Executable -Name "cmake" -Candidates @(
    (Join-Path $vsRoot "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"))

if ($cmake)
{
    Add-Check "CMake" "OK" $cmake
}
else
{
    Add-Check "CMake" "FALLO" "No encontrado; instala VS Build Tools o CMake."
}

$ctest = Find-Executable -Name "ctest" -Candidates @(
    (Join-Path $vsRoot "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"))

if ($ctest)
{
    Add-Check "CTest" "OK" $ctest
}
else
{
    Add-Check "CTest" "FALLO" "No encontrado junto a CMake."
}

if (Test-Path -LiteralPath $vsRoot)
{
    Add-Check "Visual Studio Build Tools" "OK" $vsRoot
}
else
{
    Add-Check "Visual Studio Build Tools" "FALLO" "No se encontro la ruta habitual de VS 2022."
}

$juce = Join-Path $repo "external\JUCE\CMakeLists.txt"

if (Test-Path -LiteralPath $juce)
{
    Add-Check "JUCE (submodulo)" "OK" "external\JUCE"
}
else
{
    Add-Check "JUCE (submodulo)" "FALLO" "Ejecuta: git submodule update --init --recursive"
}

foreach ($flavour in @("Dev", "Pro"))
{
    $buildDir = Get-AeonixBuildDir -Flavour $flavour
    $cache = Join-Path $buildDir "CMakeCache.txt"

    if (Test-Path -LiteralPath $cache)
    {
        Add-Check ("Build {0} configurado" -f $flavour) "OK" ((Split-Path -Leaf $buildDir) + "\")
    }
    else
    {
        Add-Check ("Build {0} configurado" -f $flavour) "AVISO" "Sin cache; el primer build ejecutara CMake."
    }

    $exe = Get-AeonixExePath -Flavour $flavour -Configuration Release

    if (Test-Path -LiteralPath $exe)
    {
        $size = (Get-Item -LiteralPath $exe).Length
        Add-Check ((Split-Path -Leaf $exe)) "OK" ("{0:N0} bytes" -f $size)
    }
    else
    {
        Add-Check ((Split-Path -Leaf $exe)) "AVISO" "No compilado todavia."
    }

    $config = Get-AeonixConfigPath -Flavour $flavour

    if (Test-Path -LiteralPath $config)
    {
        Add-Check ("Config {0}" -f $flavour) "OK" $config
    }
    else
    {
        Add-Check ("Config {0}" -f $flavour) "AVISO" "Se creara al abrir la app."
    }

    $presets = Get-AeonixPresetsDir -Flavour $flavour

    if (Test-Path -LiteralPath $presets)
    {
        $count = (Get-ChildItem -LiteralPath $presets -Filter *.json -File -ErrorAction SilentlyContinue).Count
        Add-Check ("Presets {0}" -f $flavour) "OK" ("{0} archivos en {1}" -f $count, $presets)
    }
    else
    {
        Add-Check ("Presets {0}" -f $flavour) "AVISO" "La carpeta se creara al guardar el primer preset."
    }
}

$tests = Get-AeonixTestsPath -Configuration Release -Flavour "Dev"

if (Test-Path -LiteralPath $tests)
{
    Add-Check "aeonix_tests.exe (DEV)" "OK" "Listo para ejecutar."
}
else
{
    Add-Check "aeonix_tests.exe (DEV)" "AVISO" "No compilado todavia."
}

$running = (Get-AeonixProcess).Count

if ($running -gt 0)
{
    Add-Check "AEONIX en ejecucion" "AVISO" ("{0} proceso(s); cierra la app antes de compilar." -f $running)
}
else
{
    Add-Check "AEONIX en ejecucion" "OK" "Ninguno."
}

$clangFormat = Find-Executable -Name "clang-format" -Candidates @(
    (Join-Path $vsRoot "VC\Tools\Llvm\x64\bin\clang-format.exe"))
$clangTidy = Find-Executable -Name "clang-tidy" -Candidates @(
    (Join-Path $vsRoot "VC\Tools\Llvm\x64\bin\clang-tidy.exe"))

if ($clangFormat) { Add-Check "clang-format" "OK" $clangFormat }
else { Add-Check "clang-format" "AVISO" "No disponible (se usara en el bloque de formato)." }

if ($clangTidy) { Add-Check "clang-tidy" "OK" $clangTidy }
else { Add-Check "clang-tidy" "AVISO" "No disponible (se usara en el bloque de calidad)." }

$drive = Get-PSDrive -Name C -ErrorAction SilentlyContinue

if ($drive)
{
    Add-Check "Espacio libre (C:)" "OK" ("{0:N1} GB" -f ($drive.Free / 1GB))
}

Write-Host ""
$checks | Format-Table -AutoSize

$failures = @($checks | Where-Object { $_.Estado -eq "FALLO" }).Count
$reportPath = Join-Path (Get-ReportsDir) "doctor.json"

Write-JsonFile -Path $reportPath -Object ([ordered]@{
    generatedAt = (Get-Date).ToString("o")
    ok          = ($failures -eq 0)
    failures    = $failures
    checks      = $checks.ToArray()
})

Write-Host ("  Informe: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($failures -gt 0)
{
    throw ("Doctor encontro {0} problema(s) bloqueante(s)." -f $failures)
}

Write-Host "Entorno listo." -ForegroundColor Green
