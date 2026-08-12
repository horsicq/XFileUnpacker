# XFileUnpacker smoke tests
#
# Verifies that freshly built console + GUI executables start and that the
# core unpacker functionality (detect / list / extract) works end to end
# across many archive formats. Archives are created on the fly with
# independent tools (7z.exe, makecab) as an oracle, then listed/extracted
# with xfileunpackerc and byte-compared against the original files.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File smoke_test.ps1 `
#       -BuildDir C:\tmp_build\qt5_build\xfileunpacker\build\ninja-release `
#       [-QtBinDir C:\Qt\6.10.1\msvc2022_64\bin]
#
# Exit code = number of failed tests (0 = all passed).

param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [string]$QtBinDir = "C:\Qt\6.10.1\msvc2022_64\bin",
    [string]$SevenZipExe = "C:\Program Files\7-Zip\7z.exe",
    [string]$WorkDir = ""
)

$ErrorActionPreference = 'Continue'

if ($QtBinDir -and (Test-Path $QtBinDir)) {
    $env:PATH = "$QtBinDir;$env:PATH"
}

$consoleExe = Join-Path $BuildDir "xfu_source\src\console\xfileunpackerc.exe"
$guiExe     = Join-Path $BuildDir "xfu_source\src\gui\xfileunpacker.exe"

if (-not $WorkDir) {
    $WorkDir = Join-Path $env:TEMP ("xfu_smoke_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
}
New-Item -ItemType Directory -Force $WorkDir | Out-Null

$script:nPassed = 0
$script:nFailed = 0

function Report([string]$sName, [bool]$bOk, [string]$sDetail = "") {
    if ($bOk) {
        $script:nPassed++
        Write-Output ("PASS  {0}" -f $sName)
    } else {
        $script:nFailed++
        Write-Output ("FAIL  {0}  {1}" -f $sName, $sDetail)
    }
}

function Test-SameContent([string]$sFile1, [string]$sFile2) {
    if (-not ((Test-Path $sFile1) -and (Test-Path $sFile2))) { return $false }
    $b1 = [System.IO.File]::ReadAllBytes($sFile1)
    $b2 = [System.IO.File]::ReadAllBytes($sFile2)
    if ($b1.Length -ne $b2.Length) { return $false }
    for ($i = 0; $i -lt $b1.Length; $i++) {
        if ($b1[$i] -ne $b2[$i]) { return $false }
    }
    return $true
}

# ---------------------------------------------------------------- test data
$srcDir = Join-Path $WorkDir "src"
New-Item -ItemType Directory -Force (Join-Path $srcDir "sub") | Out-Null
Set-Content (Join-Path $srcDir "hello.txt") "Hello XFileUnpacker smoke test"
Set-Content (Join-Path $srcDir "sub\nested.txt") "nested content 12345"
[byte[]]$binBytes = 0..255
[System.IO.File]::WriteAllBytes((Join-Path $srcDir "binary.bin"), $binBytes)

Report "console exe exists" (Test-Path $consoleExe) $consoleExe

$out = & $consoleExe --version 2>&1 | Out-String
Report "console --version" (($LASTEXITCODE -eq 0) -and ($out -match "xfileunpacker \d+\.\d+\.\d+")) $out.Trim()

# ---------------------------------------------------------------- archive format matrix
#
# Each entry: the archive is created with an external oracle tool, then
# xfileunpackerc must list the expected record names and extract files that
# are byte-identical to the originals.
#
# MultiFile entries pack src\* (3 files, one in a subdirectory).
# Stream entries compress exactly one payload file; extraction must
# reproduce that payload byte-identically.

$bHave7z = Test-Path $SevenZipExe
Report "7-Zip oracle available" $bHave7z $SevenZipExe

$matrix = @()

if ($bHave7z) {
    & $SevenZipExe a -y (Join-Path $WorkDir "t.zip") (Join-Path $srcDir "*") | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.7z")  (Join-Path $srcDir "*") | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.tar") (Join-Path $srcDir "*") | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.wim") (Join-Path $srcDir "*") | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.tar.gz")  (Join-Path $WorkDir "t.tar") -tgzip  | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.tar.bz2") (Join-Path $WorkDir "t.tar") -tbzip2 | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "t.tar.xz")  (Join-Path $WorkDir "t.tar") -txz    | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "hello.txt.gz")  (Join-Path $srcDir "hello.txt") -tgzip  | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "hello.txt.bz2") (Join-Path $srcDir "hello.txt") -tbzip2 | Out-Null
    & $SevenZipExe a -y (Join-Path $WorkDir "hello.txt.xz")  (Join-Path $srcDir "hello.txt") -txz    | Out-Null

    $matrix += @{ Label = "ZIP";     File = "t.zip";     Kind = "multi" }
    $matrix += @{ Label = "7Z";      File = "t.7z";      Kind = "multi" }
    $matrix += @{ Label = "TAR";     File = "t.tar";     Kind = "multi" }
    $matrix += @{ Label = "WIM";     File = "t.wim";     Kind = "multi" }
    $matrix += @{ Label = "TAR.GZ";  File = "t.tar.gz";  Kind = "stream"; Payload = "t.tar" }
    $matrix += @{ Label = "TAR.BZ2"; File = "t.tar.bz2"; Kind = "stream"; Payload = "t.tar" }
    $matrix += @{ Label = "TAR.XZ";  File = "t.tar.xz";  Kind = "stream"; Payload = "t.tar" }
    $matrix += @{ Label = "GZ";      File = "hello.txt.gz";  Kind = "stream"; Payload = "src\hello.txt" }
    $matrix += @{ Label = "BZ2";     File = "hello.txt.bz2"; Kind = "stream"; Payload = "src\hello.txt" }
    $matrix += @{ Label = "XZ";      File = "hello.txt.xz";  Kind = "stream"; Payload = "src\hello.txt" }
}

# CAB via Windows built-in makecab (single file)
makecab (Join-Path $srcDir "hello.txt") (Join-Path $WorkDir "t.cab") | Out-Null
if (Test-Path (Join-Path $WorkDir "t.cab")) {
    $matrix += @{ Label = "CAB"; File = "t.cab"; Kind = "cab" }
}

# RAR (regular + solid) via Rar.exe as an independent oracle, if present
$rarExe = "C:\Program Files\WinRAR\Rar.exe"
if (Test-Path $rarExe) {
    Push-Location $srcDir
    & $rarExe a -r -y (Join-Path $WorkDir "t.rar") "*" | Out-Null
    & $rarExe a -r -y -s -m3 (Join-Path $WorkDir "t_solid.rar") "*" | Out-Null
    Pop-Location

    $matrix += @{ Label = "RAR";       File = "t.rar";       Kind = "multi" }
    $matrix += @{ Label = "RAR-SOLID"; File = "t_solid.rar"; Kind = "multi" }
}

foreach ($entry in $matrix) {
    $label = $entry.Label
    $file = Join-Path $WorkDir $entry.File

    $bCreated = Test-Path $file
    Report "$label archive created" $bCreated $file
    if (-not $bCreated) { continue }

    # list
    $out = & $consoleExe --nocolor --listarchive $file 2>&1 | Out-String
    $bList = ($LASTEXITCODE -eq 0)
    if ($entry.Kind -eq "multi") {
        $bList = $bList -and ($out -match "hello\.txt") -and ($out -match "nested\.txt") -and ($out -match "binary\.bin")
    } elseif ($entry.Kind -eq "cab") {
        $bList = $bList -and ($out -match "hello\.txt")
    } else {
        $nRecords = ($out -split "`n" | Where-Object { $_.Trim() -ne "" }).Count - 1
        $bList = $bList -and ($nRecords -ge 1)
    }
    Report "$label list records" $bList $out.Trim()

    # extract + byte-compare
    $outDir = Join-Path $WorkDir ("out_" + ($entry.File -replace "\.", "_"))
    $out = & $consoleExe --nocolor --extractarchive $outDir $file 2>&1 | Out-String
    $bExtract = ($LASTEXITCODE -eq 0) -and ($out -match "Extracted")
    Report "$label extract runs" $bExtract $out.Trim()

    $bContent = $false
    if ($entry.Kind -eq "multi") {
        $bContent = (Test-SameContent (Join-Path $outDir "hello.txt") (Join-Path $srcDir "hello.txt")) -and
                    (Test-SameContent (Join-Path $outDir "sub\nested.txt") (Join-Path $srcDir "sub\nested.txt")) -and
                    (Test-SameContent (Join-Path $outDir "binary.bin") (Join-Path $srcDir "binary.bin"))
    } elseif ($entry.Kind -eq "cab") {
        $bContent = Test-SameContent (Join-Path $outDir "hello.txt") (Join-Path $srcDir "hello.txt")
    } else {
        # single decompressed stream: exactly one output file, byte-identical to the payload
        $extracted = @()
        if (Test-Path $outDir) { $extracted = @(Get-ChildItem -Recurse -File $outDir) }
        if ($extracted.Count -eq 1) {
            $bContent = Test-SameContent $extracted[0].FullName (Join-Path $WorkDir $entry.Payload)
        }
    }
    Report "$label extracted content byte-identical" $bContent
}

# detection sanity on a format covered by the static engine
$out = & $consoleExe --nocolor (Join-Path $WorkDir "t.zip") 2>&1 | Out-String
Report "ZIP detected by scan engine" (($LASTEXITCODE -eq 0) -and ($out -match "ZIP")) $out.Trim()

# ---------------------------------------------------------------- GUI
Report "gui exe exists" (Test-Path $guiExe) $guiExe

$bLaunch = $false
$bTitle = $false
$bShot = $false
$bClean = $false
$proc = $null
try {
    $proc = Start-Process -FilePath $guiExe -ArgumentList $WorkDir -PassThru
    $deadline = (Get-Date).AddSeconds(20)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 250
        $proc.Refresh()
        if ($proc.HasExited) { break }
        if ($proc.MainWindowHandle -ne 0) { $bLaunch = $true; break }
    }

    if ($bLaunch) {
        Start-Sleep -Seconds 3
        $proc.Refresh()
        $bTitle = ($proc.MainWindowTitle -match "XFileUnpacker")

        # occlusion-proof window capture via PrintWindow
        Add-Type -AssemblyName System.Drawing
        Add-Type @"
using System;
using System.Runtime.InteropServices;
public class XfuSmokeCapture {
    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdcBlt, uint nFlags);
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hwnd, out RECT lpRect);
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
}
"@
        $rect = New-Object XfuSmokeCapture+RECT
        [XfuSmokeCapture]::GetWindowRect($proc.MainWindowHandle, [ref]$rect) | Out-Null
        $w = $rect.Right - $rect.Left
        $h = $rect.Bottom - $rect.Top
        if (($w -gt 100) -and ($h -gt 100)) {
            $bmp = New-Object System.Drawing.Bitmap($w, $h)
            $gfx = [System.Drawing.Graphics]::FromImage($bmp)
            $hdc = $gfx.GetHdc()
            [XfuSmokeCapture]::PrintWindow($proc.MainWindowHandle, $hdc, 2) | Out-Null
            $gfx.ReleaseHdc($hdc)
            $gfx.Dispose()
            $bmp.Save((Join-Path $WorkDir "gui.png"), [System.Drawing.Imaging.ImageFormat]::Png)

            # a real window renders more than a handful of distinct colors
            $colors = New-Object 'System.Collections.Generic.HashSet[int]'
            for ($y = 10; $y -lt $h - 10; $y += [Math]::Max(1, [int]($h / 40))) {
                for ($x = 10; $x -lt $w - 10; $x += [Math]::Max(1, [int]($w / 40))) {
                    $colors.Add($bmp.GetPixel($x, $y).ToArgb()) | Out-Null
                }
            }
            $bmp.Dispose()
            $bShot = ($colors.Count -ge 4)
        }

        $proc.CloseMainWindow() | Out-Null
        $deadline = (Get-Date).AddSeconds(10)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 250
            $proc.Refresh()
            if ($proc.HasExited) { break }
        }
        $bClean = $proc.HasExited -and ($proc.ExitCode -eq 0)
    }
} finally {
    if ($proc -and (-not $proc.HasExited)) { $proc.Kill() }
}

Report "gui launches with main window" $bLaunch
Report "gui window title" $bTitle
Report "gui renders content (screenshot)" $bShot
Report "gui closes cleanly (exit 0)" $bClean

# ---------------------------------------------------------------- summary
Write-Output ""
Write-Output ("Smoke tests: {0} passed, {1} failed. Work dir: {2}" -f $script:nPassed, $script:nFailed, $WorkDir)

exit $script:nFailed
