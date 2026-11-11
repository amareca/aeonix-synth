# Ejecuta los tests del nucleo.
#
# Uso:
#   .\tools\test.ps1
#   .\tools\test.ps1 -Tag engine
#   .\tools\test.ps1 -Tag preset -Repeat 5
#   .\tools\test.ps1 -Changed
#   .\tools\test.ps1 -Changed -DryRun
#   .\tools\test.ps1 -Failed
#   .\tools\test.ps1 -Build
#
# Cada ejecucion escribe build\reports\tests.xml y tests.json.

[CmdletBinding()]
param(
    [string]$Tag = "",

    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",

    [ValidateSet("Dev", "Pro")]
    [string]$Flavour = "Dev",

    [switch]$Pro,

    [int]$Repeat = 1,

    [switch]$Changed,

    [switch]$Failed,

    [switch]$Build,

    [switch]$DryRun,

    [string[]]$Files = @()
)

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force

if ($Pro)
{
    $Flavour = "Pro"
}

$repo = Get-RepoRoot
$tests = Get-AeonixTestsPath -Configuration $Configuration -Flavour $Flavour
$reports = Get-ReportsDir
$junit = Join-Path $reports "tests.xml"
$reportJson = Join-Path $reports "tests.json"

function ConvertTo-CatchSpec
{
    param([string]$Value)

    $trimmed = $Value.Trim()

    if ($trimmed.StartsWith("["))
    {
        return $trimmed
    }

    $parts = @($trimmed -split "," | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    return (($parts | ForEach-Object { "[{0}]" -f $_ }) -join ",")
}

function Get-ChangedFiles
{
    param([string[]]$Explicit)

    if ($Explicit.Count -gt 0)
    {
        $result = New-Object System.Collections.Generic.List[string]

        foreach ($item in $Explicit)
        {
            foreach ($part in ($item -split ","))
            {
                $trimmed = $part.Trim()

                if ($trimmed)
                {
                    $result.Add($trimmed.Replace("\", "/"))
                }
            }
        }

        return @($result)
    }

    $result = New-Object System.Collections.Generic.List[string]
    $lines = @(& git -C $repo status --porcelain=v1)

    foreach ($line in $lines)
    {
        if ([string]::IsNullOrWhiteSpace($line))
        {
            continue
        }

        $path = $line.Substring(3).Trim()

        if ($path -match "^(.+) -> (.+)$")
        {
            $path = $Matches[2]
        }

        $path = $path.Trim('"').Replace("\", "/")
        $result.Add($path)
    }

    return @($result)
}

function Get-TagsForPath
{
    param([string]$Path)

    switch -Regex ($Path)
    {
        "^src/domain/Oscillator\."                     { return @("oscillator", "aliasing", "stability") }
        "^src/domain/Envelope\."                       { return @("envelope", "engine", "stability") }
        "^src/domain/Filter\."                         { return @("filter", "engine", "stability") }
        "^src/domain/Lfo\."                            { return @("lfo", "engine") }
        "^src/domain/(SynthEngine|Voice)\."            { return @("engine", "stability") }
        "^src/domain/Limiter\."                        { return @("limiter", "stability") }
        "^src/domain/Delay\."                          { return @("delay", "engine") }
        "^src/domain/Reverb\."                         { return @("reverb", "engine") }
        "^src/domain/SpScQueue\."                      { return @("queue", "engine") }
        "^src/domain/(Preset|Patch|Parameter|ParameterIds|ParameterFormat|MathUtils)\." {
            return @("preset", "patch", "parameter", "format", "math", "engine")
        }
        "^src/presentation/core/(I18n|Language|StringCatalog)" { return @("i18n") }
        "^tools/|^CONTRIBUTING\.md$|^README\.md$|^dev\.ps1$|^verify\.ps1$" {
            return @()
        }
        default                                         { return @("__all__") }
    }
}

function Get-FailedTestNames
{
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path))
    {
        return @()
    }

    try
    {
        [xml]$xml = Get-Content -LiteralPath $Path -Raw
        return @($xml.SelectNodes("//testcase[failure]") | ForEach-Object { $_.name })
    }
    catch
    {
        return @()
    }
}

function Get-TestReport
{
    param(
        [string]$Mode,
        [string]$Filter,
        [int]$ExitCode,
        [string]$Note = ""
    )

    $testCases = 0
    $failedTests = @()
    $duration = 0.0

    if (Test-Path -LiteralPath $junit)
    {
        try
        {
            [xml]$xml = Get-Content -LiteralPath $junit -Raw
            $testCases = @($xml.SelectNodes("//testcase")).Count
            $failedTests = @($xml.SelectNodes("//testcase[failure]") | ForEach-Object { $_.name })

            foreach ($suite in @($xml.SelectNodes("//testsuite")))
            {
                $duration += [double]$suite.time
            }

            if ($duration -eq 0)
            {
                foreach ($case in @($xml.SelectNodes("//testcase")))
                {
                    $duration += [double]$case.time
                }
            }
        }
        catch
        {
        }
    }

    return [ordered]@{
        generatedAt    = (Get-Date).ToString("o")
        mode           = $Mode
        filter         = $Filter
        configuration  = $Configuration
        testCases      = $testCases
        failures       = $failedTests.Count
        exitCode       = $ExitCode
        durationSeconds = [math]::Round($duration, 3)
        failedTests    = $failedTests
        note           = $Note
    }
}

if ($Changed -and $Tag)
{
    throw "Usa -Tag o -Changed, no ambos."
}

if ($Failed -and ($Tag -or $Changed))
{
    throw "Usa -Failed solo, sin -Tag ni -Changed."
}

if ($Build)
{
    & (Join-Path $PSScriptRoot "build.ps1") -Configuration $Configuration -Flavour $Flavour -Target "aeonix_tests"
}

$mode = "Full"
$filter = ""
$spec = ""
$note = ""

if ($Failed)
{
    $failedNames = @(Get-FailedTestNames -Path $junit)

    if ($failedNames.Count -eq 0)
    {
        Write-Host "No hay tests fallidos en el ultimo informe; nada que repetir." -ForegroundColor Green
        return
    }

    $mode = "Failed"
    $filter = ($failedNames -join ",")
    $spec = (($failedNames | ForEach-Object { $_ -replace '([\\\[\],~"])', '\$1' }) -join ",")
    Write-Step ("Repitiendo {0} test(s) fallido(s)" -f $failedNames.Count)
}
elseif ($Changed)
{
    $changedFiles = @(Get-ChangedFiles -Explicit $Files)
    $selected = New-Object System.Collections.Generic.List[string]
    $empty = 0

    Write-Step ("Cambios detectados: {0} archivo(s)" -f $changedFiles.Count)

    foreach ($file in $changedFiles)
    {
        $tagsForFile = @(Get-TagsForPath -Path $file)

        if ($tagsForFile.Count -eq 0)
        {
            ++$empty
            Write-Host ("  {0} -> (sin tests relevantes)" -f $file) -ForegroundColor DarkGray
            continue
        }

        if ($tagsForFile -contains "__all__")
        {
            $selected.Clear()
            $selected.Add("__all__")
            Write-Host ("  {0} -> suite completa" -f $file) -ForegroundColor Yellow
            break
        }

        Write-Host ("  {0} -> {1}" -f $file, (($tagsForFile | ForEach-Object { "[{0}]" -f $_ }) -join " "))
        foreach ($name in $tagsForFile)
        {
            if (-not $selected.Contains($name))
            {
                $selected.Add($name)
            }
        }
    }

    if ($selected.Count -eq 0)
    {
        $mode = "Changed"
        $note = "Sin tests relevantes para los archivos modificados."

        if (Test-Path -LiteralPath $junit)
        {
            Remove-Item -LiteralPath $junit -Force
        }

        Write-JsonFile -Path $reportJson -Object (Get-TestReport -Mode $mode -Filter "" -ExitCode 0 -Note $note)
        Write-Host ""
        Write-Host $note -ForegroundColor Green
        return
    }

    if ($selected.Contains("__all__"))
    {
        $mode = "Changed"
        $filter = "(suite completa)"
    }
    else
    {
        $mode = "Changed"
        $filter = (($selected | ForEach-Object { "[{0}]" -f $_ }) -join ",")
        $spec = $filter
    }
}
elseif ($Tag)
{
    $mode = "Tag"
    $filter = ConvertTo-CatchSpec -Value $Tag
    $spec = $filter
}

if ($DryRun)
{
    Write-Host ""
    Write-Host ("Modo:   {0}" -f $mode)
    Write-Host ("Filtro: {0}" -f $(if ($filter) { $filter } else { "(suite completa)" }))
    Write-Host ("Informe: {0}" -f $reportJson)
    return
}

if ($spec -and -not (Test-Path -LiteralPath $tests))
{
    throw ("No existe {0}. Compila antes con: .\dev.ps1 build -Configuration {1}" -f $tests, $Configuration)
}

if (Test-Path -LiteralPath $junit)
{
    Remove-Item -LiteralPath $junit -Force
}

$exitCode = 0
$runs = [Math]::Max(1, $Repeat)

if ($spec)
{
    for ($run = 1; $run -le $runs; ++$run)
    {
        Write-Step ("Tests {0} (pasada {1}/{2})" -f $filter, $run, $runs)
        & $tests $spec --reporter console --reporter ("junit::out={0}" -f $junit)
        $exitCode = $LASTEXITCODE

        if ($exitCode -ne 0)
        {
            break
        }
    }
}
else
{
    $ctest = Get-CTestPath
    $arguments = @(
        "--test-dir", (Join-Path $repo "build"),
        "-C", $Configuration,
        "--output-on-failure",
        "--output-junit", $junit
    )

    if ($runs -gt 1)
    {
        $arguments += @("--repeat", ("until-fail:{0}" -f $runs))
    }

    Write-Step ("Tests ({0})" -f $Configuration)
    & $ctest @arguments
    $exitCode = $LASTEXITCODE
}

$report = Get-TestReport -Mode $mode -Filter $filter -ExitCode $exitCode -Note $note
Write-JsonFile -Path $reportJson -Object $report
Write-Host ("  Informe: {0}" -f $reportJson) -ForegroundColor DarkGray

if ($exitCode -ne 0)
{
    throw ("Los tests fallaron con codigo {0}." -f $exitCode)
}

Write-Ok "Tests completados."
