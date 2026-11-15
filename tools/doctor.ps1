# Checks that the development environment is ready.
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
        Check  = $Name
        Status = $Status
        Detail = $Detail
    })
}

Write-Step "AEONIX development environment"

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
    Add-Check "CMake" "FAIL" "Not found; install VS Build Tools or CMake."
}

$ctest = Find-Executable -Name "ctest" -Candidates @(
    (Join-Path $vsRoot "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"))

if ($ctest)
{
    Add-Check "CTest" "OK" $ctest
}
else
{
    Add-Check "CTest" "FAIL" "Not found next to CMake."
}

if (Test-Path -LiteralPath $vsRoot)
{
    Add-Check "Visual Studio Build Tools" "OK" $vsRoot
}
else
{
    Add-Check "Visual Studio Build Tools" "FAIL" "The usual VS 2022 path was not found."
}

$juce = Join-Path $repo "external\JUCE\CMakeLists.txt"

if (Test-Path -LiteralPath $juce)
{
    Add-Check "JUCE (submodule)" "OK" "external\JUCE"
}
else
{
    Add-Check "JUCE (submodule)" "FAIL" "Run: git submodule update --init --recursive"
}

foreach ($flavour in @("Dev", "Pro"))
{
    $buildDir = Get-AeonixBuildDir -Flavour $flavour
    $cache = Join-Path $buildDir "CMakeCache.txt"

    if (Test-Path -LiteralPath $cache)
    {
        Add-Check ("Build {0} configured" -f $flavour) "OK" ((Split-Path -Leaf $buildDir) + "\")
    }
    else
    {
        Add-Check ("Build {0} configured" -f $flavour) "WARN" "No cache yet; the first build runs CMake."
    }

    $exe = Get-AeonixExePath -Flavour $flavour -Configuration Release

    if (Test-Path -LiteralPath $exe)
    {
        $size = (Get-Item -LiteralPath $exe).Length
        Add-Check ((Split-Path -Leaf $exe)) "OK" ("{0:N0} bytes" -f $size)
    }
    else
    {
        Add-Check ((Split-Path -Leaf $exe)) "WARN" "Not built yet."
    }

    $config = Get-AeonixConfigPath -Flavour $flavour

    if (Test-Path -LiteralPath $config)
    {
        Add-Check ("Config {0}" -f $flavour) "OK" $config
    }
    else
    {
        Add-Check ("Config {0}" -f $flavour) "WARN" "Created when the app opens."
    }

    $presets = Get-AeonixPresetsDir -Flavour $flavour

    if (Test-Path -LiteralPath $presets)
    {
        $count = (Get-ChildItem -LiteralPath $presets -Filter *.json -File -ErrorAction SilentlyContinue).Count
        Add-Check ("Presets {0}" -f $flavour) "OK" ("{0} files in {1}" -f $count, $presets)
    }
    else
    {
        Add-Check ("Presets {0}" -f $flavour) "WARN" "The folder is created when the first preset is saved."
    }
}

$tests = Get-AeonixTestsPath -Configuration Release -Flavour "Dev"

if (Test-Path -LiteralPath $tests)
{
    Add-Check "aeonix_tests.exe (DEV)" "OK" "Ready to run."
}
else
{
    Add-Check "aeonix_tests.exe (DEV)" "WARN" "Not built yet."
}

$running = (Get-AeonixProcess).Count

if ($running -gt 0)
{
    Add-Check "AEONIX running" "WARN" ("{0} process(es); close the app before building." -f $running)
}
else
{
    Add-Check "AEONIX running" "OK" "None."
}

$clangFormat = Find-Executable -Name "clang-format" -Candidates @(
    (Join-Path $vsRoot "VC\Tools\Llvm\x64\bin\clang-format.exe"))
$clangTidy = Find-Executable -Name "clang-tidy" -Candidates @(
    (Join-Path $vsRoot "VC\Tools\Llvm\x64\bin\clang-tidy.exe"))

if ($clangFormat) { Add-Check "clang-format" "OK" $clangFormat }
else { Add-Check "clang-format" "WARN" "Not available (used by the formatting block)." }

if ($clangTidy) { Add-Check "clang-tidy" "OK" $clangTidy }
else { Add-Check "clang-tidy" "WARN" "Not available (used by the quality block)." }

$drive = Get-PSDrive -Name C -ErrorAction SilentlyContinue

if ($drive)
{
    Add-Check "Free space (C:)" "OK" ("{0:N1} GB" -f ($drive.Free / 1GB))
}

Write-Host ""
$checks | Format-Table -AutoSize

$failures = @($checks | Where-Object { $_.Estado -eq "FAIL" }).Count
$reportPath = Join-Path (Get-ReportsDir) "doctor.json"

Write-JsonFile -Path $reportPath -Object ([ordered]@{
    generatedAt = (Get-Date).ToString("o")
    ok          = ($failures -eq 0)
    failures    = $failures
    checks      = $checks.ToArray()
})

Write-Host ("  Report: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($failures -gt 0)
{
    throw ("Doctor found {0} blocking problem(s)." -f $failures)
}

Write-Host "Environment ready." -ForegroundColor Green
