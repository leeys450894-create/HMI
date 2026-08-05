# Clean-Solution.ps1
# Recursively removes Visual Studio build artifact folders (x64, Debug, Release, .vs, ipch, obj)
# under the given root, while never touching the final Bin output folders
# (Source/Bin, CoreFunction/dMCCSpec/MCC/BIN, CoreFunction/EX_FDC/Bin) where the
# shipped ALPHA.exe / DLLs live.
#
# Usage:
#   .\Clean-Solution.ps1                 # scan repo root, ask for confirmation
#   .\Clean-Solution.ps1 -ListOnly        # just show what would be deleted
#   .\Clean-Solution.ps1 -Force           # delete without prompting
#   .\Clean-Solution.ps1 -WhatIf          # standard PowerShell dry-run

[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'High')]
param(
    [string]$Path = '',
    [string[]]$TargetNames = @('x64', 'x86', 'Win32', 'Debug', 'Release', '.vs', 'ipch', 'obj'),
    [string[]]$ExcludePaths = @(
        'Source\Bin',
        'CoreFunction\dMCCSpec\MCC\BIN',
        'CoreFunction\EX_FDC\Bin'
    ),
    [switch]$Force,
    [switch]$ListOnly
)

if (-not $Path) {
    if ($PSScriptRoot) {
        $Path = $PSScriptRoot
    } elseif ($MyInvocation.MyCommand.Path) {
        $Path = Split-Path -Parent $MyInvocation.MyCommand.Path
    } else {
        $Path = Get-Location
    }
}

$root = (Resolve-Path $Path).Path
$excludeFull = $ExcludePaths | ForEach-Object { Join-Path $root $_ }

Write-Host "Scanning: $root" -ForegroundColor Cyan
Write-Host "Target folder names: $($TargetNames -join ', ')" -ForegroundColor Cyan

$candidates = Get-ChildItem -Path $root -Recurse -Directory -Force -ErrorAction SilentlyContinue |
    Where-Object {
        if ($TargetNames -notcontains $_.Name) { return $false }
        if ($_.FullName -match '\\\.git(\\|$)') { return $false }
        foreach ($ex in $excludeFull) {
            if ($_.FullName -ieq $ex -or $_.FullName -ilike "$ex\*") { return $false }
        }
        return $true
    }

# Drop candidates nested inside another candidate, so we only delete the top-most folder.
$sorted = $candidates | Sort-Object { $_.FullName.Length }
$toDelete = New-Object System.Collections.Generic.List[System.IO.DirectoryInfo]
foreach ($c in $sorted) {
    $insideAnother = $toDelete | Where-Object { $c.FullName -ilike "$($_.FullName)\*" }
    if (-not $insideAnother) { $toDelete.Add($c) }
}

if ($toDelete.Count -eq 0) {
    Write-Host "No build artifact folders found." -ForegroundColor Green
    return
}

$totalSize = 0
foreach ($d in $toDelete) {
    $size = (Get-ChildItem $d.FullName -Recurse -Force -File -ErrorAction SilentlyContinue |
        Measure-Object -Property Length -Sum).Sum
    if (-not $size) { $size = 0 }
    $totalSize += $size
    $sizeMB = [math]::Round(($size / 1MB), 2)
    Write-Host ("  {0,10:N2} MB  {1}" -f $sizeMB, $d.FullName.Substring($root.Length + 1))
}
Write-Host ("Total: {0:N2} MB across {1} folder(s)" -f ($totalSize / 1MB), $toDelete.Count) -ForegroundColor Yellow

if ($ListOnly) {
    Write-Host "-ListOnly specified, nothing deleted." -ForegroundColor Cyan
    return
}

if (-not $Force -and -not $WhatIfPreference) {
    $answer = Read-Host "Delete the folders listed above? (y/N)"
    if ($answer -notmatch '^[Yy]') {
        Write-Host "Cancelled." -ForegroundColor Yellow
        return
    }
}

foreach ($d in $toDelete) {
    if ($PSCmdlet.ShouldProcess($d.FullName, "Remove-Item -Recurse -Force")) {
        try {
            Remove-Item -LiteralPath $d.FullName -Recurse -Force -ErrorAction Stop
            Write-Host "Removed: $($d.FullName)" -ForegroundColor DarkGreen
        } catch {
            Write-Warning "Failed to remove $($d.FullName): $_"
        }
    }
}

Write-Host "Done." -ForegroundColor Green
