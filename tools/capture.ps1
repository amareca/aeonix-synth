param(
    [string]$Out = "capture.png",

    [ValidateSet("main", "manager", "settings")]
    [string]$View = "manager",

    [string]$Theme = "holo.glass",

    [ValidateSet("es", "en")]
    [string]$Language = "es",

    [ValidateSet("normal", "large", "xlarge")]
    [string]$TextSize = "normal",

    [int]$Width = 1182,
    [int]$Height = 754,

    [switch]$StopExisting
)

# Opens AEONIX in a known state, captures the window and restores whatever the
# user had. The backup and restore always run, even when the capture fails.

$ErrorActionPreference = "Stop"

$repo = Split-Path -Parent $PSScriptRoot
Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force
Import-Module (Join-Path $PSScriptRoot "AeonixUi.psm1") -Force

if ($StopExisting)
{
    Stop-AeonixProcess
}
else
{
    Assert-AeonixNotRunning
}

if ([System.IO.Path]::IsPathRooted($Out))
{
    $target = $Out
}
else
{
    $target = Join-Path $repo ("out\" + $Out)
}

Backup-AeonixConfig
Backup-AeonixLibrary

try
{
    $config = New-AeonixBaseConfig -Theme $Theme -Language $Language -Preset "Default" -TextSize $TextSize
    Start-Aeonix -ConfigJson ($config | ConvertTo-Json -Compress) -Width $Width -Height $Height | Out-Null
    Start-Sleep -Milliseconds 800

    switch ($View)
    {
        "manager"
        {
            $button = Find-AeonixElementByNames -Names @("Abre el gestor de presets.", "Opens the preset manager.")

            if ($null -eq $button)
            {
                throw "The preset manager button was not found."
            }

            Invoke-AeonixElementClick -Element $button
            Start-Sleep -Milliseconds 600
        }

        "settings"
        {
            Invoke-AeonixKeys -Keys "^,"
            Start-Sleep -Milliseconds 500
        }

        "main"
        {
            Set-AeonixBackgroundFocus
        }
    }

    Save-AeonixScreenshot -Path $target

    Add-Type -AssemblyName System.Drawing
    $image = [System.Drawing.Image]::FromFile($target)

    try
    {
        Write-Host ("Capture: {0} ({1}x{2})" -f $target, $image.Width, $image.Height) -ForegroundColor Green
    }
    finally
    {
        $image.Dispose()
    }
}
finally
{
    Stop-Aeonix
    Restore-AeonixConfig
    Restore-AeonixLibrary
}
