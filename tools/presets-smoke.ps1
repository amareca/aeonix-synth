# Preset smoke test: opens the manager, searches and loads a sample.
#
# Usage:
#   .\tools\presets-smoke.ps1
#   .\tools\presets-smoke.ps1 -Queries "lead","string"
#
# Restores the user's config and library.json when it finishes.

[CmdletBinding()]
param(
    [string[]]$Queries = @("lead", "string"),

    [switch]$StopExisting
)

$ErrorActionPreference = "Stop"

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force
Import-Module (Join-Path $PSScriptRoot "AeonixUi.psm1") -Force

$Queries = @($Queries | ForEach-Object { $_ -split "," } |
             ForEach-Object { $_.Trim() } | Where-Object { $_ })

$expected = @{
    "lead"   = "Copper Lead"
    "string" = "Warm Strings"
}

$cases = @()

foreach ($query in $Queries)
{
    if (-not $expected.ContainsKey($query))
    {
        throw ("No expected preset for the query '{0}'. Add the case to the script." -f $query)
    }

    $cases += [pscustomobject]@{
        Query    = $query
        Expected = $expected[$query]
    }
}

$exe = Get-AeonixExePath -Configuration Release

if (-not (Test-Path -LiteralPath $exe))
{
    throw "AEONIX-dev.exe not found. Build it first with: .\dev.ps1 build"
}

function Open-PresetManager
{
    $button = Find-AeonixElementByNames -Names @("Abre el gestor de presets.",
                                                 "Opens the preset manager.")

    if ($null -eq $button)
    {
        throw "The preset manager button was not found."
    }

    Invoke-AeonixElementClick -Element $button
    Start-Sleep -Milliseconds 600
}

function Set-ManagerSearch
{
    param([string]$Text)

    $search = Find-AeonixElementByNames -Names @("Buscar en la biblioteca",
                                                 "Search the library")

    if ($null -eq $search)
    {
        throw "The manager search box was not found."
    }

    Invoke-AeonixElementClick -Element $search
    [System.Windows.Forms.SendKeys]::SendWait("{END}{BACKSPACE 60}")

    if ($Text)
    {
        [System.Windows.Forms.SendKeys]::SendWait(
            (ConvertTo-SendKeysLiteral -Text $Text))
    }

    Start-Sleep -Milliseconds 400
}

function Invoke-ManagerItemDoubleClick
{
    param([string]$Prefix)

    $root = Get-AeonixRootElement
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
        [System.Windows.Automation.ControlType]::ListItem)

    $item = $null

    foreach ($candidate in @($root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $condition)))
    {
        if ($candidate.Current.Name -like "$Prefix*")
        {
            $item = $candidate
            break
        }
    }

    if ($null -eq $item)
    {
        return $false
    }

    $rect = $item.Current.BoundingRectangle
    $x = [int]($rect.X + $rect.Width / 2)
    $y = [int]($rect.Y + $rect.Height / 2)

    [void][AeonixUiNative]::SetCursorPos($x, $y)
    Start-Sleep -Milliseconds 100

    for ($click = 0; $click -lt 2; ++$click)
    {
        [AeonixUiNative]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 50
        [AeonixUiNative]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 80
    }

    Start-Sleep -Milliseconds 500
    return $true
}

$results = New-Object System.Collections.Generic.List[object]
$failures = 0

Backup-AeonixConfig
Backup-AeonixLibrary

try
{
    $config = New-AeonixBaseConfig -Theme "holo.glass" -Language "es" -Preset "Default" -TextSize "large"
    Start-Aeonix -ConfigJson ($config | ConvertTo-Json -Compress) -StopExisting:$StopExisting | Out-Null
    Start-Sleep -Milliseconds 1200

    foreach ($case in $cases)
    {
        Open-PresetManager
        Set-ManagerSearch -Text $case.Query

        $found = Invoke-ManagerItemDoubleClick -Prefix $case.Expected
        $loaded = $false

        if ($found)
        {
            $loaded = Wait-AeonixConfigValue -Key "preset.last" -Expected $case.Expected -TimeoutSeconds 5
        }
        else
        {
            Invoke-AeonixKeys -Keys "{ESC}"
        }

        if (-not $loaded)
        {
            ++$failures
        }

        $results.Add([pscustomobject]@{
            Search   = $case.Query
            Expected = $case.Expected
            Loaded   = Get-AeonixConfigValue -Key "preset.last"
            Status   = if ($loaded) { "OK" } else { "FAIL" }
        })
    }
}
finally
{
    Stop-Aeonix
    Restore-AeonixConfig
    Restore-AeonixLibrary
}

Write-Host ""
$results | Format-Table -AutoSize

$reportPath = Join-Path (Get-ReportsDir) "presets.json"

Write-JsonFile -Path $reportPath -Object ([ordered]@{
    generatedAt = (Get-Date).ToString("o")
    ok          = ($failures -eq 0)
    failures    = $failures
    cases       = $results.ToArray()
})

Write-Host ("  Report: {0}" -f $reportPath) -ForegroundColor DarkGray

if ($failures -gt 0)
{
    throw ("Preset smoke test with {0} failure(s)." -f $failures)
}

Write-Host ("Presets OK ({0} cases)." -f $results.Count) -ForegroundColor Green
