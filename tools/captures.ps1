# Resumen y limpieza de las capturas F11/F12.
#
# Uso:
#   .\tools\captures.ps1
#   .\tools\captures.ps1 -Open
#   .\tools\captures.ps1 -Legacy
#   .\tools\captures.ps1 -Clean
#   .\tools\captures.ps1 -Clean -Force
#
# -Legacy mueve los ficheros antiguos de la raiz a _legacy\YYYY-MM-DD\.
# -Clean borra todo el contenido de la carpeta de capturas.

[CmdletBinding()]
param(
    [switch]$Clean,

    [switch]$Legacy,

    [switch]$Open,

    [switch]$Force,

    [string]$Root = ""
)

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force

if ($Clean -and $Legacy)
{
    throw "Usa -Clean o -Legacy, no ambos."
}

if (-not $Root)
{
    $Root = Get-AeonixSnapshotDir
}

$Root = [System.IO.Path]::GetFullPath($Root)
$allowedRoot = [System.IO.Path]::GetFullPath((Join-Path $env:TEMP "Aeonix"))
$allowedPrefix = $allowedRoot.TrimEnd("\") + "\"

if (-not $Root.StartsWith($allowedPrefix, [System.StringComparison]::OrdinalIgnoreCase))
{
    throw ("Por seguridad, la carpeta debe estar dentro de {0}." -f $allowedRoot)
}

function Get-CaptureSummary
{
    param([string]$Directory)

    $rows = New-Object System.Collections.Generic.List[object]
    $totalFiles = 0
    $totalBytes = [int64]0

    if (Test-Path -LiteralPath $Directory)
    {
        foreach ($folder in (Get-ChildItem -LiteralPath $Directory -Directory -ErrorAction SilentlyContinue |
                             Sort-Object Name))
        {
            $files = @(Get-ChildItem -LiteralPath $folder.FullName -File -Recurse -ErrorAction SilentlyContinue)
            $bytes = [int64](($files | Measure-Object Length -Sum).Sum)
            $totalFiles += $files.Count
            $totalBytes += $bytes

            $rows.Add([pscustomobject]@{
                Feature = $folder.Name
                Archivos = $files.Count
                MB = [math]::Round($bytes / 1MB, 2)
            })
        }

        $rootFiles = @(Get-ChildItem -LiteralPath $Directory -File -ErrorAction SilentlyContinue)
        $rootBytes = [int64](($rootFiles | Measure-Object Length -Sum).Sum)
        $totalFiles += $rootFiles.Count
        $totalBytes += $rootBytes

        if ($rootFiles.Count -gt 0)
        {
            $rows.Add([pscustomobject]@{
                Feature = "(raiz)"
                Archivos = $rootFiles.Count
                MB = [math]::Round($rootBytes / 1MB, 2)
            })
        }
    }

    return [pscustomobject]@{
        Rows = $rows.ToArray()
        TotalFiles = $totalFiles
        TotalMB = [math]::Round($totalBytes / 1MB, 2)
    }
}

function Move-LegacyCaptures
{
    param([string]$Directory)

    if (-not (Test-Path -LiteralPath $Directory))
    {
        return 0
    }

    $files = @(Get-ChildItem -LiteralPath $Directory -File -ErrorAction SilentlyContinue |
               Where-Object { $_.Name -like "snapshot-*" -or $_.Name -like "layout-*" })

    if ($files.Count -eq 0)
    {
        return 0
    }

    $destination = Join-Path $Directory ("_legacy\{0}" -f (Get-Date -Format "yyyy-MM-dd"))
    New-Item -ItemType Directory -Path $destination -Force | Out-Null

    $moved = 0

    foreach ($file in $files)
    {
        $target = Join-Path $destination $file.Name

        if (Test-Path -LiteralPath $target)
        {
            $baseName = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
            $extension = $file.Extension
            $index = 1

            do
            {
                $target = Join-Path $destination ("{0}-{1}{2}" -f $baseName, $index, $extension)
                ++$index
            }
            while (Test-Path -LiteralPath $target)
        }

        Move-Item -LiteralPath $file.FullName -Destination $target
        ++$moved
    }

    return $moved
}

function Clear-Captures
{
    param(
        [string]$Directory,
        [bool]$SkipConfirmation
    )

    if (-not (Test-Path -LiteralPath $Directory))
    {
        Write-Host "No hay capturas que borrar." -ForegroundColor Yellow
        return
    }

    if (-not $SkipConfirmation)
    {
        $answer = Read-Host ("Se borrara todo el contenido de {0}. Escribe SI para continuar" -f $Directory)

        if ($answer -ne "SI")
        {
            Write-Host "Cancelado." -ForegroundColor Yellow
            return
        }
    }

    Get-ChildItem -LiteralPath $Directory -Force -ErrorAction SilentlyContinue |
        Remove-Item -Recurse -Force

    Write-Host ("Capturas borradas: {0}" -f $Directory) -ForegroundColor Green
}

Write-Step "Capturas de AEONIX"

if ($Legacy)
{
    $moved = Move-LegacyCaptures -Directory $Root
    Write-Host ("  Movidos a _legacy: {0} archivos" -f $moved) -ForegroundColor Green
}

if ($Clean)
{
    Clear-Captures -Directory $Root -SkipConfirmation:([bool]$Force)
}

$summary = Get-CaptureSummary -Directory $Root

if ($summary.Rows.Count -gt 0)
{
    $summary.Rows | Format-Table -AutoSize
}
else
{
    Write-Host "  No hay capturas." -ForegroundColor Yellow
}

Write-Host ("  Total: {0} archivos, {1:N2} MB" -f $summary.TotalFiles, $summary.TotalMB)
Write-Host ("  Carpeta: {0}" -f $Root) -ForegroundColor DarkGray

$reportPath = Join-Path (Get-ReportsDir) "captures.json"

Write-JsonFile -Path $reportPath -Object ([ordered]@{
    generatedAt = (Get-Date).ToString("o")
    root = $Root
    features = $summary.Rows
    totalFiles = $summary.TotalFiles
    totalMB = $summary.TotalMB
})

Write-Host ("  Informe: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($Open)
{
    if (-not (Test-Path -LiteralPath $Root))
    {
        New-Item -ItemType Directory -Path $Root -Force | Out-Null
    }

    Start-Process -FilePath $Root
}
