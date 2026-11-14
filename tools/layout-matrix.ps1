# Visual check matrix: themes x text sizes x window sizes x languages.
#
# Usage:
#   .\tools\layout-matrix.ps1
#   .\tools\layout-matrix.ps1 -Quick
#   .\tools\layout-matrix.ps1 -TextSizes large
#   .\tools\layout-matrix.ps1 -Sizes 1180x752
#   .\tools\layout-matrix.ps1 -OutputDir .\docs\assets\snapshots -NoSnapshot
#
# -Quick is the iteration profile: main theme, Spanish, large text and the
# configured window (1180x752 of content). It writes layout-quick.json and does
# not replace the layout.json of the full matrix. Explicit parameters take
# priority over the profile.
#
# The matrix starts the app once per language and text size. Within each run it
# switches themes with F5-F8 and captures with F10 (PNG + report) or F11. It
# restores the user config when it finishes, even if something fails.

[CmdletBinding()]
param(
    [string[]]$Languages = @("es", "en"),

    [string[]]$Themes = @("holo.accents", "holo.glass", "holo.minimal", "prototype"),

    [string[]]$TextSizes = @("normal", "large", "xlarge"),

    # When given, it is used for every text size.
    [string[]]$Sizes = @(),

    # Iteration profile: holo.accents, es, large and 1180x752.
    [switch]$Quick,

    [string]$OutputDir = "",

    [switch]$NoSnapshot,

    [switch]$StopExisting
)

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force
Import-Module (Join-Path $PSScriptRoot "AeonixUi.psm1") -Force

function Expand-List
{
    param([string[]]$Values)

    return @($Values | ForEach-Object { $_ -split "," } |
             ForEach-Object { $_.Trim() } | Where-Object { $_ })
}

$Languages = Expand-List -Values $Languages
$Themes    = Expand-List -Values $Themes
$TextSizes = Expand-List -Values $TextSizes
$Sizes     = Expand-List -Values $Sizes

# Quick profile for iterating the interface. It only applies to parameters that
# were not passed explicitly.
if ($Quick)
{
    $explicitTextSizes = $PSBoundParameters.ContainsKey("TextSizes")

    if (-not $PSBoundParameters.ContainsKey("Languages")) { $Languages = @("es") }
    if (-not $PSBoundParameters.ContainsKey("Themes"))    { $Themes = @("holo.accents") }
    if (-not $explicitTextSizes)                          { $TextSizes = @("large") }

    if (-not $PSBoundParameters.ContainsKey("Sizes"))
    {
        # Without -TextSizes the user's configured window is used. When other text
        # sizes are requested, each one keeps its recommended window so Extra large
        # is never measured below its minimum.
        $Sizes = if ($explicitTextSizes) { @() } else { @("1180x752") }
    }

    if (-not $OutputDir)
    {
        $NoSnapshot = $true
    }
}

function ConvertFrom-SizeString
{
    param([string]$Text)

    if ($Text -notmatch "^(\d+)x(\d+)$")
    {
        throw ("Invalid size: {0}. Use the 1180x752 format." -f $Text)
    }

    return [pscustomobject]@{
        Width  = [int]$Matches[1]
        Height = [int]$Matches[2]
    }
}

$sizeTable = @{
    "normal" = @("1180x752", "1040x712")
    "large"  = @("1220x770", "1160x724")
    "xlarge" = @("1320x790", "1300x744")
}

foreach ($textSize in $TextSizes)
{
    if (-not $sizeTable.ContainsKey($textSize))
    {
        throw ("Invalid text size: {0}. Use normal, large or xlarge." -f $textSize)
    }
}

if ($Languages.Count -eq 0 -or $Themes.Count -eq 0 -or $TextSizes.Count -eq 0)
{
    throw "The matrix needs at least one language, one theme and one text size."
}

$exe = Get-AeonixExePath -Configuration Release

if (-not (Test-Path -LiteralPath $exe))
{
    throw "AEONIX-dev.exe not found. Build it first with: .\dev.ps1 build"
}

$results = New-Object System.Collections.Generic.List[object]
$failures = 0
$captureTag = "layout-matrix"
$previousCaptureTag = $env:AEONIX_CAPTURE_TAG
$previousMatrixFlag = $env:AEONIX_CAPTURE_MATRIX
$matrixDir = Get-AeonixSnapshotDir -Tag $captureTag
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

Backup-AeonixConfig
Backup-AeonixLibrary

try
{
    $env:AEONIX_CAPTURE_TAG = $captureTag
    $env:AEONIX_CAPTURE_MATRIX = "1"

    New-Item -ItemType Directory -Path $matrixDir -Force | Out-Null
    Get-ChildItem -LiteralPath $matrixDir -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "matrix-*" -or $_.Name -like "layout-*" -or $_.Name -like "snapshot-*" } |
        Remove-Item -Force

    foreach ($language in $Languages)
    {
        foreach ($textSize in $TextSizes)
        {
            $sizesForLevel = if ($Sizes.Count -gt 0) { $Sizes } else { $sizeTable[$textSize] }
            $config = New-AeonixBaseConfig -Theme $Themes[0] `
                                           -Language $language `
                                           -Preset "Default" `
                                           -TextSize $textSize

            Start-Aeonix -ConfigJson ($config | ConvertTo-Json -Compress) `
                         -WaitMilliseconds 250 `
                         -StopExisting:$StopExisting | Out-Null
            Set-AeonixFocus

            try
            {
                foreach ($theme in $Themes)
                {
                    $themeName = $theme.Replace(".", "-")
                    Invoke-AeonixThemeKey -Theme $theme

                    foreach ($sizeText in $sizesForLevel)
                    {
                        $size = ConvertFrom-SizeString -Text $sizeText
                        $stem = "matrix-{0}-{1}-{2}-{3}" -f $language, $themeName, $textSize, $sizeText
                        $layoutPath = Join-Path $matrixDir ($stem + ".txt")
                        $picturePath = Join-Path $matrixDir ($stem + ".png")
                        $captureBoth = (-not $NoSnapshot) -or [bool]$OutputDir
                        $attempt = 0
                        $summary = $null
                        $picture = $null

                        while ($attempt -lt 2)
                        {
                            ++$attempt
                            Set-AeonixWindowSize -Width ($size.Width + 2) `
                                                 -Height ($size.Height + 2) `
                                                 -TimeoutMilliseconds 500
                            Start-Sleep -Milliseconds 30

                            if ($attempt -gt 1)
                            {
                                Set-AeonixFocus
                                Invoke-AeonixThemeKey -Theme $theme
                            }

                            $since = (Get-Date).AddMilliseconds(-500)

                            if ($captureBoth)
                            {
                                if ($attempt -eq 1)
                                {
                                    Invoke-AeonixCaptureKey
                                }
                                else
                                {
                                    # Fallback when F10 does not reach the main component.
                                    Invoke-AeonixSnapshotKey
                                    Invoke-AeonixLayoutKey
                                }
                            }
                            else
                            {
                                Invoke-AeonixLayoutKey
                            }

                            $layout = Wait-AeonixFile -Path $layoutPath `
                                                      -Since $since `
                                                      -TimeoutMilliseconds 2000

                            if ($captureBoth)
                            {
                                $picture = Wait-AeonixFile -Path $picturePath `
                                                           -Since $since `
                                                           -TimeoutMilliseconds 2000
                            }

                            if ($null -ne $layout)
                            {
                                $header = @(Get-Content -LiteralPath $layout.FullName -TotalCount 5)
                                $themeOk = $header.Count -ge 5 -and $header[1] -eq ("Theme: " + $theme)
                                $languageOk = $header.Count -ge 5 -and $header[2] -eq ("Language: " + $language)
                                $textSizeOk = $header.Count -ge 5 -and $header[3] -eq ("Text size: " + $textSize)
                                $sizeOk = $header.Count -ge 5 -and
                                          $header[4] -eq ("MainComponent: {0} x {1}" -f $size.Width, $size.Height)

                                if ($themeOk -and $languageOk -and $textSizeOk -and $sizeOk)
                                {
                                    $line = Get-Content -LiteralPath $layout.FullName | Select-Object -Last 1
                                    $summary = ConvertFrom-AeonixLayoutSummary -Line $line
                                    break
                                }
                            }

                            Start-Sleep -Milliseconds 100
                        }

                        if ($null -eq $summary)
                        {
                            throw ("Could not capture {0}." -f $stem)
                        }

                        if ($captureBoth -and $null -eq $picture)
                        {
                            throw ("No PNG was generated for {0}." -f $stem)
                        }

                        if ($OutputDir -and $null -ne $picture)
                        {
                            $screenshot = Join-Path $OutputDir (
                                "dev-{0}-{1}-{2}-{3}.png" -f $language, $themeName, $textSize, $sizeText)
                            $screenshotDir = Split-Path -Parent $screenshot

                            if ($screenshotDir -and -not (Test-Path -LiteralPath $screenshotDir))
                            {
                                New-Item -ItemType Directory -Path $screenshotDir -Force | Out-Null
                            }

                            Copy-Item -LiteralPath $picture.FullName -Destination $screenshot -Force
                        }

                        if (-not $summary.Ok)
                        {
                            ++$failures
                        }

                        $results.Add([pscustomobject]@{
                            Language  = $language
                            Theme     = $theme
                            Text      = $textSize
                            Size      = $sizeText
                            Controls  = $summary.Controls
                            Overlaps  = $summary.Overlaps
                            Outside   = $summary.Outside
                            Clipped   = $summary.Clipped
                            Status    = if ($summary.Ok) { "OK" } else { "FAIL" }
                        })

                        Write-Host ("{0}/{1}/{2} {3}: {4}" -f $language, $theme, $textSize, $sizeText, $summary.Line)
                    }
                }
            }
            finally
            {
                Stop-Aeonix
            }
        }
    }
}
finally
{
    Stop-Aeonix
    Restore-AeonixConfig
    Restore-AeonixLibrary

    if ($null -eq $previousCaptureTag)
    {
        Remove-Item -Path Env:\AEONIX_CAPTURE_TAG -ErrorAction SilentlyContinue
    }
    else
    {
        $env:AEONIX_CAPTURE_TAG = $previousCaptureTag
    }

    if ($null -eq $previousMatrixFlag)
    {
        Remove-Item -Path Env:\AEONIX_CAPTURE_MATRIX -ErrorAction SilentlyContinue
    }
    else
    {
        $env:AEONIX_CAPTURE_MATRIX = $previousMatrixFlag
    }
}

$stopwatch.Stop()

Write-Host ""
$results | Format-Table -AutoSize

$mode = if ($Quick) { "quick" } else { "full" }
$reportName = if ($Quick) { "layout-quick.json" } else { "layout.json" }
$reportPath = Join-Path (Get-ReportsDir) $reportName

$report = [ordered]@{
    generatedAt  = (Get-Date).ToString("o")
    mode         = $mode
    ok           = ($failures -eq 0)
    failures     = $failures
    combinations = $results.ToArray()
}

if ($Quick)
{
    $report["quickProfile"] = [ordered]@{
        languages = ($Languages -join ",")
        themes    = ($Themes -join ",")
        textSizes = ($TextSizes -join ",")
        sizes     = if ($Sizes.Count -gt 0) { $Sizes -join "," } else { "auto" }
    }
}

Write-JsonFile -Path $reportPath -Object $report

Write-Host ("  Report: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($failures -gt 0)
{
    throw ("Layout with {0} combination(s) with problems." -f $failures)
}

Write-Host ("Layout OK in {0} combinations in {1:N1} s." -f $results.Count, $stopwatch.Elapsed.TotalSeconds) -ForegroundColor Green

if ($Quick)
{
    Write-Host ("  Quick ({0} combination(s)): the full matrix is still pending before closing the change." -f $results.Count) -ForegroundColor Yellow
}
