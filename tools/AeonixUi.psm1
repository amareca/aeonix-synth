# Automatizacion de la interfaz de AEONIX para pruebas manuales repetibles.
#
# Estrategia hibrida:
#   - Botones y ventana: UI Automation (por nombre de accesibilidad).
#   - Campos de texto: los dos Edit del navegador de presets, ordenados por
#     posicion (el superior es el buscador). JUCE no expone su nombre por UIA.
#   - Teclas y clics: Win32 como respaldo.
#
# Todas las funciones trabajan sobre la ultima sesion iniciada con Start-Aeonix.
# Los scripts que usan este modulo deben hacer Backup-AeonixConfig antes y
# Restore-AeonixConfig en un bloque finally.

Import-Module (Join-Path $PSScriptRoot "Common.psm1") -Force

if (-not ("AeonixUiNative" -as [type]))
{
    Add-Type @"
using System;
using System.Runtime.InteropServices;

public struct AeonixUiRect
{
    public int Left;
    public int Top;
    public int Right;
    public int Bottom;
}

public static class AeonixUiNative
{
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out AeonixUiRect rect);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int cx, int cy, uint flags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int command);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extra);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
"@
}

[void][AeonixUiNative]::SetProcessDPIAware()

Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes

$script:Session = $null

function Get-AeonixSession
{
    if ($null -eq $script:Session)
    {
        throw "No hay sesion de AEONIX. Llama antes a Start-Aeonix."
    }

    return $script:Session
}

function Wait-AeonixWindow
{
    param(
        [System.Diagnostics.Process]$Process,
        [int]$TimeoutSeconds = 20
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-Date) -lt $deadline)
    {
        $Process.Refresh()

        if ($Process.MainWindowHandle -ne [IntPtr]::Zero)
        {
            return $Process.MainWindowHandle
        }

        Start-Sleep -Milliseconds 250
    }

    throw "AEONIX no abrio su ventana a tiempo."
}

function Start-Aeonix
{
    param(
        [string]$ExePath = "",
        [string]$ConfigJson = "",
        [ValidateSet("Dev", "Pro")]
        [string]$Flavour = "Dev",
        [int]$Width = 1182,
        [int]$Height = 754,
        [int]$X = 100,
        [int]$Y = 90,
        [switch]$NoResize,
        [switch]$StopExisting,
        [int]$WaitMilliseconds = 1500
    )

    if (-not $ExePath)
    {
        $ExePath = Get-AeonixExePath -Configuration Release -Flavour $Flavour
    }

    if (-not (Test-Path -LiteralPath $ExePath))
    {
        throw ("No existe {0}. Compila antes con: .\dev.ps1 build" -f $ExePath)
    }

    if ((Get-AeonixProcess).Count -gt 0)
    {
        if (-not $StopExisting)
        {
            throw "AEONIX esta en ejecucion; cierra la app o pasa -StopExisting."
        }

        Stop-AeonixProcess
    }

    if ($ConfigJson)
    {
        Set-AeonixConfigJson -Json $ConfigJson -Flavour $Flavour
    }

    $process = Start-Process -FilePath $ExePath -PassThru
    $handle = Wait-AeonixWindow -Process $process

    if (-not $NoResize)
    {
        # SWP_NOZORDER | SWP_SHOWWINDOW
        [void][AeonixUiNative]::SetWindowPos($handle, [IntPtr]::Zero, $X, $Y, $Width, $Height, 0x0004 -bor 0x0040)
    }

    [void][AeonixUiNative]::SetForegroundWindow($handle)
    Start-Sleep -Milliseconds $WaitMilliseconds

    $script:Session = [pscustomobject]@{
        Process = $process
        Handle  = $handle
    }

    Connect-AeonixWindow

    return $script:Session
}

function Stop-Aeonix
{
    param([int]$TimeoutSeconds = 5)

    $process = $null

    if ($null -ne $script:Session)
    {
        $process = $script:Session.Process
    }

    foreach ($candidate in (Get-AeonixProcess))
    {
        try { [void]$candidate.CloseMainWindow() } catch { }
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-AeonixProcess).Count -gt 0 -and (Get-Date) -lt $deadline)
    {
        Start-Sleep -Milliseconds 50
    }

    foreach ($candidate in (Get-AeonixProcess))
    {
        Stop-Process -Id $candidate.Id -Force -ErrorAction SilentlyContinue
    }

    $script:Session = $null
}

function Get-AeonixWindowRect
{
    $session = Get-AeonixSession
    $rect = New-Object AeonixUiRect

    if (-not [AeonixUiNative]::GetWindowRect($session.Handle, [ref]$rect))
    {
        throw "No se pudo leer la posicion de la ventana de AEONIX."
    }

    return [pscustomobject]@{
        Left   = $rect.Left
        Top    = $rect.Top
        Right  = $rect.Right
        Bottom = $rect.Bottom
        Width  = $rect.Right - $rect.Left
        Height = $rect.Bottom - $rect.Top
    }
}

function Set-AeonixWindowSize
{
    param(
        [int]$Width,
        [int]$Height,
        [int]$TimeoutMilliseconds = 500
    )

    $session = Get-AeonixSession
    $rect = Get-AeonixWindowRect

    if ($rect.Width -eq $Width -and $rect.Height -eq $Height)
    {
        return
    }

    # SWP_NOZORDER
    [void][AeonixUiNative]::SetWindowPos($session.Handle, [IntPtr]::Zero, $rect.Left, $rect.Top, $Width, $Height, 0x0004)

    $deadline = (Get-Date).AddMilliseconds($TimeoutMilliseconds)

    while ((Get-Date) -lt $deadline)
    {
        Start-Sleep -Milliseconds 20
        $rect = Get-AeonixWindowRect

        if ($rect.Width -eq $Width -and $rect.Height -eq $Height)
        {
            return
        }
    }

    throw ("La ventana no alcanzo {0}x{1} a tiempo." -f $Width, $Height)
}

function Connect-AeonixWindow
{
    $session = Get-AeonixSession
    $handle = $session.Handle

    if ([AeonixUiNative]::GetForegroundWindow() -eq $handle)
    {
        return
    }

    # SW_RESTORE
    [void][AeonixUiNative]::ShowWindow($handle, 9)
    [void][AeonixUiNative]::SetForegroundWindow($handle)

    if ([AeonixUiNative]::GetForegroundWindow() -ne $handle)
    {
        # Windows puede bloquear SetForegroundWindow si el proceso no tiene el
        # foco. El truco TOPMOST/NOTOPMOST suele desbloquearlo sin robar el foco
        # de forma permanente.
        [void][AeonixUiNative]::SetWindowPos($handle, [IntPtr](-1), 0, 0, 0, 0, 0x0001 -bor 0x0002 -bor 0x0010)
        [void][AeonixUiNative]::SetWindowPos($handle, [IntPtr](-2), 0, 0, 0, 0, 0x0001 -bor 0x0002 -bor 0x0010)
        [void][AeonixUiNative]::SetForegroundWindow($handle)
    }

    if ([AeonixUiNative]::GetForegroundWindow() -ne $handle)
    {
        try { [Microsoft.VisualBasic.Interaction]::AppActivate($session.Process.Id) } catch { }
    }

    Start-Sleep -Milliseconds 250
}

function Invoke-AeonixKeys
{
    param([string]$Keys)

    Connect-AeonixWindow
    [System.Windows.Forms.SendKeys]::SendWait($Keys)
    Start-Sleep -Milliseconds 150
}

function ConvertTo-SendKeysLiteral
{
    param([string]$Text)

    return [regex]::Replace($Text, '([+^%~(){}\[\]])', '{$1}')
}

function Get-AeonixRootElement
{
    $session = Get-AeonixSession
    return [System.Windows.Automation.AutomationElement]::FromHandle($session.Handle)
}

function Find-AeonixElement
{
    param([string]$Name)

    $root = Get-AeonixRootElement
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, $Name)

    return $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $condition)
}

function Find-AeonixElementByNames
{
    param([string[]]$Names)

    foreach ($name in $Names)
    {
        $element = Find-AeonixElement -Name $name

        if ($null -ne $element)
        {
            return $element
        }
    }

    return $null
}

function Find-AeonixCombo
{
    param([string]$Name)

    $root = Get-AeonixRootElement
    $nameCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, $Name)
    $typeCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
        [System.Windows.Automation.ControlType]::ComboBox)

    return $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants,
                           (New-Object System.Windows.Automation.AndCondition($nameCondition, $typeCondition)))
}

function Select-AeonixComboItem
{
    param(
        [string]$Name,
        [string]$Item
    )

    $combo = Find-AeonixCombo -Name $Name

    if ($null -eq $combo)
    {
        throw ("No se encontro el combo '{0}'." -f $Name)
    }

    Connect-AeonixWindow
    $combo.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
    Start-Sleep -Milliseconds 400

    $all = [System.Windows.Automation.AutomationElement]::RootElement.FindAll(
        [System.Windows.Automation.TreeScope]::Descendants,
        [System.Windows.Automation.Condition]::TrueCondition)

    foreach ($element in $all)
    {
        if (($element.Current.Name -eq $Item) -and
            ($element.Current.ControlType -eq [System.Windows.Automation.ControlType]::MenuItem))
        {
            try
            {
                $element.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
            }
            catch
            {
                Invoke-AeonixElementClick -Element $element
            }

            Start-Sleep -Milliseconds 300
            return $true
        }
    }

    return $false
}

function Invoke-AeonixElementClick
{
    param([System.Windows.Automation.AutomationElement]$Element)

    Connect-AeonixWindow

    $rect = $Element.Current.BoundingRectangle
    $x = [int]($rect.X + $rect.Width / 2)
    $y = [int]($rect.Y + $rect.Height / 2)

    [void][AeonixUiNative]::SetCursorPos($x, $y)
    Start-Sleep -Milliseconds 120
    [AeonixUiNative]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 60
    [AeonixUiNative]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 250
}

function Invoke-AeonixButton
{
    param([string]$Name)

    $element = Find-AeonixElement -Name $Name

    if ($null -eq $element)
    {
        throw ("No se encontro el boton '{0}' por accesibilidad." -f $Name)
    }

    try
    {
        $pattern = $element.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
        $pattern.Invoke()
        Start-Sleep -Milliseconds 250
    }
    catch
    {
        Invoke-AeonixElementClick -Element $element
    }
}

function Get-AeonixEditElements
{
    $root = Get-AeonixRootElement
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
        [System.Windows.Automation.ControlType]::Edit)

    $edits = @($root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $condition))

    # Los editores se ordenan por posicion: el buscador del gestor queda arriba.
    return @($edits | Sort-Object { $_.Current.BoundingRectangle.Top }, { $_.Current.BoundingRectangle.Left })
}

function Get-AeonixSearchEdit
{
    $byName = Find-AeonixElementByNames -Names @("Buscar en la biblioteca",
                                                 "Search the library",
                                                 "Buscar preset",
                                                 "Search preset")

    if ($null -ne $byName)
    {
        return $byName
    }

    $edits = Get-AeonixEditElements

    if ($edits.Count -lt 1)
    {
        throw "No se encontro el buscador de presets."
    }

    return $edits[0]
}

function Get-AeonixPresetCombo
{
    return Find-AeonixElementByNames -Names @("Selector de preset", "Preset selector")
}

function Get-AeonixThemeCombo
{
    return Find-AeonixElementByNames -Names @("Selector de tema", "Theme selector")
}

function Get-AeonixPolyphonyCombo
{
    return Find-AeonixElementByNames -Names @("Polyphony")
}

function Set-AeonixSearchText
{
    param([string]$Text = "")

    $session = Get-AeonixSession
    [void][AeonixUiNative]::SetForegroundWindow($session.Handle)

    $search = Get-AeonixSearchEdit

    # Un clic garantiza el foco real (UIA SetFocus no siempre funciona en JUCE).
    Invoke-AeonixElementClick -Element $search
    Start-Sleep -Milliseconds 120

    # Los backspaces vacian el campo; END garantiza el final antes de borrar.
    [System.Windows.Forms.SendKeys]::SendWait("{END}")
    [System.Windows.Forms.SendKeys]::SendWait("{BACKSPACE 60}")

    if ($Text)
    {
        [System.Windows.Forms.SendKeys]::SendWait((ConvertTo-SendKeysLiteral -Text $Text))
    }

    Start-Sleep -Milliseconds 250
}

function Send-AeonixEnter   { Invoke-AeonixKeys -Keys "{ENTER}" }
function Send-AeonixEscape  { Invoke-AeonixKeys -Keys "{ESC}" }

function Invoke-AeonixFunctionKey
{
    param(
        [byte]$VirtualKey,
        [int]$SettleMilliseconds = 400
    )

    Connect-AeonixWindow
    [AeonixUiNative]::keybd_event($VirtualKey, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 20
    [AeonixUiNative]::keybd_event($VirtualKey, 0, 2, [UIntPtr]::Zero)

    if ($SettleMilliseconds -gt 0)
    {
        Start-Sleep -Milliseconds $SettleMilliseconds
    }
}

function Invoke-AeonixLayoutKey   { Invoke-AeonixFunctionKey -VirtualKey 0x7A }   # F11
function Invoke-AeonixSnapshotKey { Invoke-AeonixFunctionKey -VirtualKey 0x7B }   # F12
function Invoke-AeonixCaptureKey  { Invoke-AeonixFunctionKey -VirtualKey 0x79 -SettleMilliseconds 20 } # F10

function Invoke-AeonixThemeKey
{
    param(
        [ValidateSet("holo.accents", "holo.glass", "holo.minimal", "prototype")]
        [string]$Theme
    )

    $keys = @{
        "holo.accents" = 0x74   # F5
        "holo.glass"   = 0x75   # F6
        "holo.minimal" = 0x76   # F7
        "prototype"    = 0x77   # F8
    }

    Invoke-AeonixFunctionKey -VirtualKey $keys[$Theme] -SettleMilliseconds 30
}

function Wait-AeonixFile
{
    param(
        [string]$Path,
        [datetime]$Since,
        [int]$TimeoutMilliseconds = 2000,
        [int]$IntervalMilliseconds = 25
    )

    $deadline = (Get-Date).AddMilliseconds($TimeoutMilliseconds)

    while ((Get-Date) -lt $deadline)
    {
        if (Test-Path -LiteralPath $Path)
        {
            $item = Get-Item -LiteralPath $Path

            if ($item.LastWriteTime -ge $Since)
            {
                return $item
            }
        }

        Start-Sleep -Milliseconds $IntervalMilliseconds
    }

    return $null
}

function Get-NewestAeonixFile
{
    param(
        [string]$Filter,
        [datetime]$Since,
        [string]$Tag = ""
    )

    if ($Tag)
    {
        $directory = Get-AeonixSnapshotDir -Tag $Tag

        if (-not (Test-Path -LiteralPath $directory))
        {
            return $null
        }

        return Get-ChildItem -LiteralPath $directory -Filter $Filter -File |
            Where-Object { $_.LastWriteTime -ge $Since } |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 1
    }

    $directory = Get-AeonixSnapshotDir

    if (-not (Test-Path -LiteralPath $directory))
    {
        return $null
    }

    return Get-ChildItem -LiteralPath $directory -Filter $Filter -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object {
            $_.LastWriteTime -ge $Since -and
            $_.FullName -notmatch "\\_legacy\\" -and
            $_.FullName -notmatch "\\archive\\"
        } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

function Save-AeonixScreenshot
{
    param([string]$Path)

    Connect-AeonixWindow
    $rect = Get-AeonixWindowRect
    $bitmap = New-Object System.Drawing.Bitmap $rect.Width, $rect.Height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)

    try
    {
        $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
        $directory = Split-Path -Parent $Path

        if ($directory -and -not (Test-Path -LiteralPath $directory))
        {
            New-Item -ItemType Directory -Path $directory -Force | Out-Null
        }

        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally
    {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function ConvertFrom-AeonixLayoutSummary
{
    param([string]$Line)

    if ($Line -match "Resumen:\s*(\d+)\s+controles,\s*(\d+)\s+solapes,\s*(\d+)\s+fuera,\s*(\d+)\s+textos cortados")
    {
        return [pscustomobject]@{
            Line     = $Line
            Controls = [int]$Matches[1]
            Overlaps = [int]$Matches[2]
            Outside  = [int]$Matches[3]
            Clipped  = [int]$Matches[4]
            Ok       = ([int]$Matches[2] -eq 0 -and [int]$Matches[3] -eq 0 -and [int]$Matches[4] -eq 0)
        }
    }

    throw ("Formato de informe de layout no reconocido: {0}" -f $Line)
}

function Set-AeonixFocus
{
    param([int]$SettleMilliseconds = 50)

    Connect-AeonixWindow

    $rect = Get-AeonixWindowRect

    # Zona neutra de la barra superior (etiqueta de estado): quita el foco de
    # cualquier control para que F5-F12 lleguen al componente principal.
    $x = $rect.Left + [int]($rect.Width / 2)
    $y = $rect.Top + 60

    [void][AeonixUiNative]::SetCursorPos($x, $y)
    Start-Sleep -Milliseconds 20
    [AeonixUiNative]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 20
    [AeonixUiNative]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds $SettleMilliseconds
}

function Set-AeonixBackgroundFocus
{
    Set-AeonixFocus -SettleMilliseconds 250
}

function Invoke-AeonixLayoutReport
{
    param(
        [switch]$Snapshot,
        [string]$ScreenshotPath = "",
        [string]$Tag = ""
    )

    Set-AeonixBackgroundFocus
    $since = (Get-Date).AddSeconds(-1)

    if ($Snapshot)
    {
        Invoke-AeonixSnapshotKey
    }

    Invoke-AeonixLayoutKey
    Start-Sleep -Milliseconds 500

    $layout = Get-NewestAeonixFile -Filter "layout-*.txt" -Since $since -Tag $Tag
    $picture = $null

    if ($null -eq $layout)
    {
        Start-Sleep -Seconds 1
        $layout = Get-NewestAeonixFile -Filter "layout-*.txt" -Since $since -Tag $Tag
    }

    if ($null -eq $layout)
    {
        throw "AEONIX no genero el informe de layout (F11)."
    }

    if ($Snapshot)
    {
        $picture = Get-NewestAeonixFile -Filter "snapshot-*.png" -Since $since -Tag $Tag

        if ($null -eq $picture)
        {
            Start-Sleep -Seconds 1
            $picture = Get-NewestAeonixFile -Filter "snapshot-*.png" -Since $since -Tag $Tag
        }
    }

    if ($ScreenshotPath)
    {
        if ($picture)
        {
            $directory = Split-Path -Parent $ScreenshotPath

            if ($directory -and -not (Test-Path -LiteralPath $directory))
            {
                New-Item -ItemType Directory -Path $directory -Force | Out-Null
            }

            Copy-Item -LiteralPath $picture.FullName -Destination $ScreenshotPath -Force
        }
        else
        {
            Save-AeonixScreenshot -Path $ScreenshotPath
        }
    }

    $line = Get-Content -LiteralPath $layout.FullName | Select-Object -Last 1
    $summary = ConvertFrom-AeonixLayoutSummary -Line $line

    return [pscustomobject]@{
        Summary    = $summary
        LayoutFile = $layout.FullName
        Screenshot = if ($picture) { $picture.FullName } else { $null }
        Size       = [System.IO.Path]::GetFileNameWithoutExtension($layout.Name)
    }
}

function Get-AeonixConfigValue
{
    param([string]$Key)

    $config = Read-AeonixConfig

    if ($null -eq $config)
    {
        return $null
    }

    $property = $config.PSObject.Properties[$Key]

    if ($null -eq $property)
    {
        return $null
    }

    return $property.Value
}

function Wait-AeonixConfigValue
{
    param(
        [string]$Key,
        [string]$Expected,
        [int]$TimeoutSeconds = 5
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)

    while ((Get-Date) -lt $deadline)
    {
        $value = Get-AeonixConfigValue -Key $Key

        if ($null -ne $value -and [string]$value -eq $Expected)
        {
            return $true
        }

        Start-Sleep -Milliseconds 200
    }

    return $false
}

Export-ModuleMember -Function *
