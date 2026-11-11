# Matriz de comprobacion visual: temas x tamanos de texto x tamanos x idiomas.
#
# Uso:
#   .\tools\layout-matrix.ps1
#   .\tools\layout-matrix.ps1 -Quick
#   .\tools\layout-matrix.ps1 -TextSizes large
#   .\tools\layout-matrix.ps1 -Sizes 1180x752
#   .\tools\layout-matrix.ps1 -OutputDir .\docs\assets\snapshots -NoSnapshot
#
# -Quick es el perfil de iteracion: tema principal, espanol, texto Grande y la
# ventana configurada (1180x752 de contenido). Escribe layout-quick.json y no
# sustituye al layout.json de la matriz completa. Los parametros explicitos
# tienen prioridad sobre el perfil.
#
# La matriz arranca una sola vez por idioma y tamano de texto. Dentro de cada
# arranque cambia de tema con F5-F8 y captura con F10 (PNG + informe) o F11.
# Restaura la config del usuario al terminar, incluso si algo falla.

[CmdletBinding()]
param(
    [string[]]$Languages = @("es", "en"),

    [string[]]$Themes = @("holo.accents", "holo.glass", "holo.minimal", "prototype"),

    [string[]]$TextSizes = @("normal", "large", "xlarge"),

    # Si se indica, se usa para todos los tamanos de texto.
    [string[]]$Sizes = @(),

    # Perfil de iteracion: holo.accents, es, large y 1180x752.
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

# Perfil rapido para iterar la interfaz. Solo se aplica a los parametros que no
# se hayan pasado de forma explicita.
if ($Quick)
{
    $explicitTextSizes = $PSBoundParameters.ContainsKey("TextSizes")

    if (-not $PSBoundParameters.ContainsKey("Languages")) { $Languages = @("es") }
    if (-not $PSBoundParameters.ContainsKey("Themes"))    { $Themes = @("holo.accents") }
    if (-not $explicitTextSizes)                          { $TextSizes = @("large") }

    if (-not $PSBoundParameters.ContainsKey("Sizes"))
    {
        # Sin -TextSizes se usa la ventana configurada del usuario. Si se piden
        # otros tamanos de texto, se dejan las ventanas recomendadas de cada uno
        # para no medir Muy grande por debajo de su minimo.
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
        throw ("Tamano no valido: {0}. Usa el formato 1180x752." -f $Text)
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
        throw ("Tamano de texto no valido: {0}. Usa normal, large o xlarge." -f $textSize)
    }
}

if ($Languages.Count -eq 0 -or $Themes.Count -eq 0 -or $TextSizes.Count -eq 0)
{
    throw "La matriz necesita al menos un idioma, un tema y un tamano de texto."
}

$exe = Get-AeonixExePath -Configuration Release

if (-not (Test-Path -LiteralPath $exe))
{
    throw "No existe AEONIX-dev.exe. Compila antes con: .\dev.ps1 build"
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
                                    # Respaldo si F10 no llega al componente principal.
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
                            throw ("No se pudo capturar {0}." -f $stem)
                        }

                        if ($captureBoth -and $null -eq $picture)
                        {
                            throw ("No se genero el PNG de {0}." -f $stem)
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
                            Idioma    = $language
                            Tema      = $theme
                            Texto     = $textSize
                            Tamano    = $sizeText
                            Controles = $summary.Controls
                            Solapes   = $summary.Overlaps
                            Fuera     = $summary.Outside
                            Cortados  = $summary.Clipped
                            Estado    = if ($summary.Ok) { "OK" } else { "FALLO" }
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

Write-Host ("  Informe: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($failures -gt 0)
{
    throw ("Layout con {0} combinacion(es) con problemas." -f $failures)
}

Write-Host ("Layout OK en {0} combinaciones en {1:N1} s." -f $results.Count, $stopwatch.Elapsed.TotalSeconds) -ForegroundColor Green

if ($Quick)
{
    Write-Host ("  Rapido ({0} combinacion(es)): la matriz completa sigue pendiente antes de cerrar el cambio." -f $results.Count) -ForegroundColor Yellow
}
