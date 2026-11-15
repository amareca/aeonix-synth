param(
    [string]$Out = "capture.png",

    [ValidateSet("main", "manager", "settings")]
    [string]$View = "manager",

    [string]$Theme = "holo.glass",

    [ValidateSet("es", "en")]
    [string]$Language = "es",

    [ValidateSet("normal", "large", "xlarge")]
    [string]$TextSize = "normal",

    [ValidateSet("Dev", "Pro")]
    [string]$Flavour = "Dev",

    [string]$Search = "",

    [switch]$EmptyLibrary,

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

Backup-AeonixConfig -Flavour $Flavour
Backup-AeonixLibrary -Flavour $Flavour

$stashDir = ""
$presetsDir = Get-AeonixPresetsDir -Flavour $Flavour

if ($EmptyLibrary)
{
    # Marketing shots show the factory library and nothing else: the user's own
    # presets would leak into the capture. They are moved aside, not deleted, and
    # the finally block puts them back.
    $libraryPath = Get-AeonixLibraryPath -Flavour $Flavour
    $libraryJson = '{"version":1,"factory":{},"user":{}}'
    [System.IO.File]::WriteAllText($libraryPath, $libraryJson, (New-Object System.Text.UTF8Encoding($false)))

    $userPresets = @(Get-ChildItem -LiteralPath $presetsDir -Filter *.json -File -ErrorAction SilentlyContinue)

    if ($userPresets.Count -gt 0)
    {
        $stashDir = Join-Path ([System.IO.Path]::GetTempPath()) ("aeonix-capture-" + [guid]::NewGuid().ToString("N"))
        New-Item -ItemType Directory -Path $stashDir -Force | Out-Null

        foreach ($preset in $userPresets)
        {
            Move-Item -LiteralPath $preset.FullName -Destination (Join-Path $stashDir $preset.Name) -Force
        }
    }
}

try
{
    $config = New-AeonixBaseConfig -Theme $Theme -Language $Language -Preset "Default" -TextSize $TextSize
    Start-Aeonix -ConfigJson ($config | ConvertTo-Json -Compress) -Flavour $Flavour -Width $Width -Height $Height | Out-Null
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

            if ($Search)
            {
                Set-AeonixSearchText -Text $Search
                Start-Sleep -Milliseconds 500

                # Select the first result so the detail matches the filtered list.
                $root = Get-AeonixRootElement
                $condition = New-Object System.Windows.Automation.PropertyCondition(
                    [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
                    [System.Windows.Automation.ControlType]::ListItem)
                $first = @($root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $condition)) | Select-Object -First 1

                if ($null -ne $first)
                {
                    Invoke-AeonixElementClick -Element $first
                    Start-Sleep -Milliseconds 400
                }
            }
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

    if ($stashDir -and (Test-Path -LiteralPath $stashDir))
    {
        foreach ($preset in (Get-ChildItem -LiteralPath $stashDir -Filter *.json -File))
        {
            Move-Item -LiteralPath $preset.FullName -Destination (Join-Path $presetsDir $preset.Name) -Force
        }

        Remove-Item -LiteralPath $stashDir -Force -Recurse
    }

    Restore-AeonixConfig -Flavour $Flavour
    Restore-AeonixLibrary -Flavour $Flavour
}
