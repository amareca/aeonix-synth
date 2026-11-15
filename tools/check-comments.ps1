param(
    [string]$Base = "HEAD",
    [string]$Repo = ""
)

# Comment-only verifier: strips every // comment from both the base revision and
# the working tree and compares the remaining code. Used to prove that a sweep
# touched prose only, so a 900-line diff can be reviewed without reading it line
# by line.

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)

if (-not $Repo -or $Repo.Length -eq 0)
{
    $Repo = Split-Path -Parent $PSScriptRoot
}

Push-Location $Repo

try
{
    $files = @(& git diff --name-only --diff-filter=M $Base -- src tests 2>$null)

    if ($files.Count -eq 0)
    {
        "No modified files under src/ or tests/ against $Base."
        exit 0
    }

    function Get-CodeOnly([string]$source)
    {
        $builder = New-Object System.Text.StringBuilder
        $i = 0
        $n = $source.Length
        $inString = $false
        $inChar = $false

        while ($i -lt $n)
        {
            $c = $source[$i]

            if ($inString)
            {
                [void]$builder.Append($c)
                if ($c -eq [char]'\' -and ($i + 1) -lt $n) { $i++; [void]$builder.Append($source[$i]) }
                elseif ($c -eq [char]'"') { $inString = $false }
                $i++
                continue
            }

            if ($inChar)
            {
                [void]$builder.Append($c)
                if ($c -eq [char]'\' -and ($i + 1) -lt $n) { $i++; [void]$builder.Append($source[$i]) }
                elseif ($c -eq [char]"'") { $inChar = $false }
                $i++
                continue
            }

            if ($c -eq [char]'"') { $inString = $true; [void]$builder.Append($c); $i++; continue }
            if ($c -eq [char]"'") { $inChar = $true; [void]$builder.Append($c); $i++; continue }

            if ($c -eq [char]'/' -and ($i + 1) -lt $n -and $source[$i + 1] -eq [char]'/')
            {
                while ($i -lt $n -and $source[$i] -ne [char]"`n") { $i++ }
                continue
            }

            [void]$builder.Append($c)
            $i++
        }

        $text = $builder.ToString()
        $text = $text -replace "`r`n", "`n"
        $text = $text -replace "[`t ]+`n", "`n"

        return $text.TrimEnd()
    }

    $failures = 0
    $total = 0

    foreach ($file in $files)
    {
        $total++
        $oldText = (@(& git show ("{0}:{1}" -f $Base, $file)) -join "`n")
        $newText = [System.IO.File]::ReadAllText((Join-Path $Repo $file))

        $oldCode = Get-CodeOnly $oldText
        $newCode = Get-CodeOnly $newText

        if ($oldCode -ne $newCode)
        {
            $failures++
            "CODE CHANGED: $file"

            $oldLines = $oldCode -split "`n"
            $newLines = $newCode -split "`n"
            $limit = [Math]::Max($oldLines.Count, $newLines.Count)

            for ($k = 0; $k -lt $limit; $k++)
            {
                $a = if ($k -lt $oldLines.Count) { $oldLines[$k] } else { "<missing>" }
                $b = if ($k -lt $newLines.Count) { $newLines[$k] } else { "<missing>" }

                if ($a -ne $b)
                {
                    "  line {0}:" -f ($k + 1)
                    "    before: $a"
                    "    after:  $b"
                    break
                }
            }
        }
    }

    ""
    "Files checked: $total. Files with code changes: $failures."

    if ($failures -gt 0) { exit 1 }
}
finally
{
    Pop-Location
}
