# Compatibilidad: el flujo real vive en tools\verify.ps1.
# Recomendado: .\dev.ps1 verify

$ErrorActionPreference = "Stop"

$target = Join-Path $PSScriptRoot "tools\verify.ps1"

try
{
    & $target @args
}
catch
{
    Write-Host ""
    Write-Host ("Error: {0}" -f $_.Exception.Message) -ForegroundColor Red
    exit 1
}
