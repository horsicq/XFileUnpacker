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
#       [-QtBinDir C:\Qt\6.10.1\msvc2022_64\bin] `
#       [-CabCorpusDir D:\files\cab]
#
# Exit code = number of failed tests (0 = all passed).

param(
    [Parameter(Mandatory=$true)][string]$BuildDir,
    [string]$QtBinDir = "C:\Qt\6.10.1\msvc2022_64\bin",
    [string]$SevenZipExe = "C:\Program Files\7-Zip\7z.exe",
    [string]$WorkDir = "",
    [string]$CabCorpusDir = $env:XFU_CAB_CORPUS
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

function Get-DirectoryManifest([string]$sRoot) {
    $result = @()
    if (-not (Test-Path -LiteralPath $sRoot -PathType Container)) { return $result }

    $rootPath = [System.IO.Path]::GetFullPath($sRoot).TrimEnd([char[]]"\/")
    foreach ($file in @(Get-ChildItem -LiteralPath $rootPath -Recurse -File | Sort-Object FullName)) {
        $relativePath = $file.FullName.Substring($rootPath.Length).TrimStart([char[]]"\/") -replace '/', '\'
        $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        $result += ("{0}`t{1}`t{2}" -f $relativePath, $file.Length, $hash)
    }

    return @($result | Sort-Object)
}

function Compare-DirectoryContent([string]$sReferenceRoot, [string]$sActualRoot) {
    $reference = @(Get-DirectoryManifest $sReferenceRoot)
    $actual = @(Get-DirectoryManifest $sActualRoot)

    return [PSCustomObject]@{
        Same = (($reference.Count -eq $actual.Count) -and
                (($reference -join "`n") -ceq ($actual -join "`n")))
        Detail = ("reference files={0}, actual files={1}" -f $reference.Count, $actual.Count)
        ReferenceCount = $reference.Count
        ActualCount = $actual.Count
    }
}

function Get-GuiArchiveState([IntPtr]$hWnd, [string]$sArchiveFileName) {
    $state = [ordered]@{
        RecordsLoaded = $false
        CommandsEnabled = $false
        GridRows = 0
        StatusRows = 0
        ObservedNames = @()
    }

    try {
        Add-Type -AssemblyName UIAutomationClient
        Add-Type -AssemblyName UIAutomationTypes

        $root = [System.Windows.Automation.AutomationElement]::FromHandle($hWnd)
        if (-not $root) { return [PSCustomObject]$state }

        $elements = $root.FindAll(
            [System.Windows.Automation.TreeScope]::Descendants,
            [System.Windows.Automation.Condition]::TrueCondition)

        $bExtractAll = $false
        $bTest = $false
        $bAdvanced = $false
        $bHello = $false
        $bBinary = $false
        $bNested = $false

        foreach ($element in $elements) {
            try {
                $name = $element.Current.Name
                $automationId = $element.Current.AutomationId
                $bEnabled = $element.Current.IsEnabled

                if ($name -match '^([1-9][0-9]*) record\(s\)$') {
                    $state.StatusRows = [Math]::Max($state.StatusRows, [int]$Matches[1])
                }

                if ($name -match '(^|[\\/])hello\.txt$') {
                    $bHello = $true
                    $state.ObservedNames += $name
                } elseif ($name -match '(^|[\\/])binary\.bin$') {
                    $bBinary = $true
                    $state.ObservedNames += $name
                } elseif ($name -match '(^|[\\/])nested\.txt$') {
                    $bNested = $true
                    $state.ObservedNames += $name
                }

                if ((($name -eq 'Extract all') -or ($automationId -eq 'toolButtonExtractAll')) -and $bEnabled) { $bExtractAll = $true }
                if ((($name -eq 'Test') -or ($automationId -eq 'toolButtonTest')) -and $bEnabled) { $bTest = $true }
                if ((($name -eq 'Advanced') -or ($automationId -eq 'checkBoxAdvanced')) -and $bEnabled) { $bAdvanced = $true }

                $gridPatternObject = $null
                if ($element.TryGetCurrentPattern([System.Windows.Automation.GridPattern]::Pattern, [ref]$gridPatternObject)) {
                    $gridPattern = [System.Windows.Automation.GridPattern]$gridPatternObject
                    $state.GridRows = [Math]::Max($state.GridRows, $gridPattern.Current.RowCount)
                }
            } catch {
                # An element may disappear while Qt updates the model; retry on the next poll.
            }
        }

        $state.CommandsEnabled = $bExtractAll -and $bTest -and $bAdvanced

        if ((Split-Path $sArchiveFileName -Leaf) -eq 't.zip') {
            $state.RecordsLoaded = ($bHello -and $bBinary -and $bNested) -or
                                   ($state.GridRows -ge 3) -or ($state.StatusRows -ge 3)
        } elseif ((Split-Path $sArchiveFileName -Leaf) -eq '1.cab') {
            $state.RecordsLoaded = ($state.GridRows -ge 8) -or ($state.StatusRows -ge 8)
        } else {
            $state.RecordsLoaded = $bHello -or ($state.GridRows -ge 1) -or ($state.StatusRows -ge 1)
        }
    } catch {
        # The caller reports a failed semantic GUI check with the other smoke results.
    }

    return [PSCustomObject]$state
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

# ---------------------------------------------------------------- external CAB corpus
#
# This optional corpus is deliberately fingerprinted because it contains a
# mixture of clean cabinets, corrupt headers/data, and data-valid archives
# whose doubled path separators must be rejected by the safe extraction path.
# Set -CabCorpusDir (or XFU_CAB_CORPUS) to enable these cases.

if ($CabCorpusDir) {
    $cabCorpusCases = @(
        @{ Name = "1.cab";        Bytes = 66717526; Sha256 = "09b6050ea8cd0ee3e8db3ef844e6caf5e2e887177bd600a18d3ea2acdce40fec"; OracleValid = $false; List = $true;  Extract = $false; Records = 8 },
        @{ Name = "1ex.cab";      Bytes = 168;      Sha256 = "c99f85c09b32071b52a3e69a2d01029d136a325a171c956a2145f59b40fb0c91"; OracleValid = $true;  List = $true;  Extract = $true;  Records = 1 },
        @{ Name = "2.cab";        Bytes = 85054295; Sha256 = "f82596493377a8c9e6d66c972324a23b748b2ef855855c5df43a533af1c89c4f"; OracleValid = $false; List = $false; Extract = $false; Records = 0 },
        @{ Name = "3.cab";        Bytes = 96770493; Sha256 = "a67d031f266b1c1d7e44183fde0d0e52ce8259d62ac33d71cd4fb7dd3d213117"; OracleValid = $false; List = $false; Extract = $false; Records = 0 },
        @{ Name = "a.cab";        Bytes = 85;       Sha256 = "c0d2dc2adc85c7482ea4f41baa30ea4397e344eca829c930bb9715049908d9a9"; OracleValid = $true;  List = $true;  Extract = $true;  Records = 1 },
        @{ Name = "LZX.cab";      Bytes = 29425;    Sha256 = "207d1405b8f6379882d87d2321c8e8ccc13dc77372764a45b9b4bb353b0f3544"; OracleValid = $true;  List = $true;  Extract = $true;  Records = 1 },
        @{ Name = "mpdf-000.cab"; Bytes = 29527;    Sha256 = "3ef761b1f8e88771385b1f755adb99d7a55b308aaf358d6fef2f61420b250057"; OracleValid = $true;  List = $true;  Extract = $true;  Records = 1 },
        @{ Name = "out.cab";      Bytes = 24209478; Sha256 = "4fe5557bd2ab2f5554a9de84467f244c3a6587b6a9d88f311b0795646a3d6c14"; OracleValid = $true;  List = $true;  Extract = $false; Records = 31 },
        @{ Name = "out2.cab";     Bytes = 3230006;  Sha256 = "c31a9ed0da680ee5aab129132c5ed0ef00fb00d7f28819f5968a7833e08ec1ac"; OracleValid = $true;  List = $true;  Extract = $false; Records = 4 },
        @{ Name = "out3.cab";     Bytes = 136;      Sha256 = "a1e57c98c1fe96a6807fbad3d7073aecc359422f6591da7e787f9b198fd6b946"; OracleValid = $true;  List = $true;  Extract = $false; Records = 3 }
    )

    $bCabCorpusExists = Test-Path -LiteralPath $CabCorpusDir -PathType Container
    Report "CAB corpus directory exists" $bCabCorpusExists $CabCorpusDir

    if ($bCabCorpusExists) {
        $cabCorpusRoot = [System.IO.Path]::GetFullPath($CabCorpusDir).TrimEnd([char[]]"\/")
        $actualCabNames = @(Get-ChildItem -LiteralPath $cabCorpusRoot -Filter *.cab -File | Sort-Object Name | ForEach-Object { $_.Name })
        $expectedCabNames = @($cabCorpusCases | ForEach-Object { $_.Name } | Sort-Object)
        $bCabFileSet = (($actualCabNames.Count -eq $expectedCabNames.Count) -and
                        (($actualCabNames -join "`n") -ceq ($expectedCabNames -join "`n")))
        Report "CAB corpus file set" $bCabFileSet ("expected={0}, actual={1}" -f $expectedCabNames.Count, $actualCabNames.Count)

        if (-not $bHave7z) {
            Report "CAB corpus 7-Zip oracle available" $false $SevenZipExe
        } else {
            $cabCorpusRunDir = Join-Path $WorkDir ("cab_corpus_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
            New-Item -ItemType Directory -Force $cabCorpusRunDir | Out-Null

            $caseIndex = 0
            foreach ($case in $cabCorpusCases) {
                $caseIndex++
                $cabFile = Join-Path $cabCorpusRoot $case.Name
                $label = "CAB corpus " + $case.Name
                $bInputExists = Test-Path -LiteralPath $cabFile -PathType Leaf
                $bFingerprint = $false

                if ($bInputExists) {
                    $cabItem = Get-Item -LiteralPath $cabFile
                    $cabHash = (Get-FileHash -LiteralPath $cabFile -Algorithm SHA256).Hash.ToLowerInvariant()
                    $bFingerprint = (($cabItem.Length -eq $case.Bytes) -and ($cabHash -ceq $case.Sha256))
                }
                Report "$label input fingerprint" $bFingerprint $cabFile
                if (-not $bFingerprint) { continue }

                $caseDir = Join-Path $cabCorpusRunDir ("{0:D2}_{1}" -f $caseIndex, ($case.Name -replace '[^A-Za-z0-9._-]', '_'))
                $oracleDir = Join-Path $caseDir "oracle"
                $actualDir = Join-Path $caseDir "actual"
                New-Item -ItemType Directory -Force $caseDir | Out-Null

                $oracleTestOutput = & $SevenZipExe t -y -- $cabFile 2>&1 | Out-String
                $oracleTestCode = $LASTEXITCODE
                $bOracleDisposition = (($oracleTestCode -eq 0) -eq [bool]$case.OracleValid)
                Report "$label oracle disposition" $bOracleDisposition $oracleTestOutput.Trim()

                $listOutput = & $consoleExe --nocolor --listarchive $cabFile 2>&1 | Out-String
                $listCode = $LASTEXITCODE
                if ($case.List) {
                    $recordPattern = "CAB:\s+{0}\s+file\(s\)" -f $case.Records
                    $bListDisposition = ($listCode -eq 0) -and ($listOutput -match $recordPattern)
                } else {
                    $bListDisposition = ($listCode -eq 2)
                }
                Report "$label list disposition" $bListDisposition $listOutput.Trim()

                if ($case.Extract) {
                    $oracleExtractOutput = & $SevenZipExe x -y ("-o" + $oracleDir) -- $cabFile 2>&1 | Out-String
                    $oracleExtractCode = $LASTEXITCODE
                    $extractOutput = & $consoleExe --nocolor --extractarchive $actualDir $cabFile 2>&1 | Out-String
                    $extractCode = $LASTEXITCODE
                    $bExtractDisposition = ($extractCode -eq 0) -and ($extractOutput -match "Extracted")
                    Report "$label extract disposition" $bExtractDisposition $extractOutput.Trim()

                    $comparison = Compare-DirectoryContent $oracleDir $actualDir
                    $bContent = ($oracleExtractCode -eq 0) -and $bExtractDisposition -and $comparison.Same
                    $contentDetail = ("{0}; oracle: {1}" -f $comparison.Detail, $oracleExtractOutput.Trim())
                    Report "$label extracted content matches oracle" $bContent $contentDetail
                } else {
                    New-Item -ItemType Directory -Force $actualDir | Out-Null
                    $extractOutput = & $consoleExe --nocolor --extractarchive $actualDir $cabFile 2>&1 | Out-String
                    $extractCode = $LASTEXITCODE
                    $outputChildren = @(Get-ChildItem -LiteralPath $actualDir -Force -Recurse -ErrorAction SilentlyContinue)
                    $bExtractDisposition = ($extractCode -eq 2) -and ($outputChildren.Count -eq 0)
                    $extractDetail = ("exit={0}, output entries={1}; {2}" -f $extractCode, $outputChildren.Count, $extractOutput.Trim())
                    Report "$label extract rejection leaves no output" $bExtractDisposition $extractDetail
                }
            }
        }
    }
}

# detection sanity on a format covered by the static engine
$zipFile = Join-Path $WorkDir "t.zip"
if (Test-Path $zipFile) {
    $out = & $consoleExe --nocolor $zipFile 2>&1 | Out-String
    Report "ZIP detected by scan engine" (($LASTEXITCODE -eq 0) -and ($out -match "ZIP")) $out.Trim()
}

# ---------------------------------------------------------------- GUI
Report "gui exe exists" (Test-Path $guiExe) $guiExe

$guiStageDir = Join-Path $WorkDir ("gui_portable_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $guiStageDir | Out-Null
$guiSmokeExe = Join-Path $guiStageDir "xfileunpacker.exe"
Copy-Item -LiteralPath $guiExe -Destination $guiSmokeExe -Force
New-Item -ItemType File -Force (Join-Path $guiStageDir "portable") | Out-Null
@("[View]", "Lang=", "Qss=", "Style=Fusion") | Set-Content -Encoding ASCII (Join-Path $guiStageDir "xfileunpacker.ini")

$bLaunch = $false
$bTitle = $false
$bArchiveRecords = $false
$bArchiveCommands = $false
$bShot = $false
$bClean = $false
$proc = $null
$guiState = [PSCustomObject]@{ GridRows = 0; StatusRows = 0; ObservedNames = @() }
$guiArchive = ""
if ($CabCorpusDir) {
    $cabGuiCandidate = Join-Path $CabCorpusDir "1.cab"
    if (Test-Path -LiteralPath $cabGuiCandidate -PathType Leaf) { $guiArchive = $cabGuiCandidate }
}
if (-not $guiArchive) { $guiArchive = Join-Path $WorkDir "t.zip" }
if (-not (Test-Path -LiteralPath $guiArchive -PathType Leaf)) { $guiArchive = Join-Path $WorkDir "t.cab" }
try {
    $previousAppData = $env:APPDATA
    $previousLocalAppData = $env:LOCALAPPDATA
    $smokeAppData = Join-Path $WorkDir "appdata"
    $smokeLocalAppData = Join-Path $WorkDir "localappdata"
    New-Item -ItemType Directory -Force $smokeAppData, $smokeLocalAppData | Out-Null

    try {
        $env:APPDATA = $smokeAppData
        $env:LOCALAPPDATA = $smokeLocalAppData
        $proc = Start-Process -FilePath $guiSmokeExe -ArgumentList ('"' + $guiArchive + '"') -PassThru
    } finally {
        $env:APPDATA = $previousAppData
        $env:LOCALAPPDATA = $previousLocalAppData
    }

    $deadline = (Get-Date).AddSeconds(20)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 250
        $proc.Refresh()
        if ($proc.HasExited) { break }
        if ($proc.MainWindowHandle -ne 0) { $bLaunch = $true; break }
    }

    if ($bLaunch) {
        $deadline = (Get-Date).AddSeconds(20)
        while ((Get-Date) -lt $deadline) {
            $guiState = Get-GuiArchiveState $proc.MainWindowHandle $guiArchive
            $bArchiveRecords = $guiState.RecordsLoaded
            $bArchiveCommands = $guiState.CommandsEnabled
            if ($bArchiveRecords -and $bArchiveCommands) { break }
            Start-Sleep -Milliseconds 250
        }

        $proc.Refresh()
        $bTitle = ($proc.MainWindowTitle -match "XFileUnpacker") -and
                  ($proc.MainWindowTitle -match [Regex]::Escape((Split-Path $guiArchive -Leaf)))

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
Report "gui archive records loaded" $bArchiveRecords ("grid={0}; status={1}; names={2}" -f $guiState.GridRows, $guiState.StatusRows, ($guiState.ObservedNames -join ','))
Report "gui archive commands enabled" $bArchiveCommands
Report "gui renders content (screenshot)" $bShot
Report "gui closes cleanly (exit 0)" $bClean

# ---------------------------------------------------------------- summary
Write-Output ""
Write-Output ("Smoke tests: {0} passed, {1} failed. Work dir: {2}" -f $script:nPassed, $script:nFailed, $WorkDir)

exit $script:nFailed
