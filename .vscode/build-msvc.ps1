$ErrorActionPreference = 'Stop'

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw 'vswhere.exe not found. Install Visual Studio Build Tools.'
}

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw 'MSVC C++ tools not found. Install Desktop development with C++ workload.'
}

$devCmd = Join-Path $vsPath 'Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path $devCmd)) {
    throw 'VsDevCmd.bat not found.'
}

$workspace = $PSScriptRoot | Split-Path -Parent
$exe = Join-Path $workspace 'main.exe'
$pdb = Join-Path $workspace 'main.pdb'
$tmpCmd = Join-Path $env:TEMP 'cursor_inventory_build.cmd'

$sources = Get-ChildItem -Path $workspace -Filter *.cpp -Recurse -File |
    Where-Object {
        $_.FullName -notmatch '\\\.vscode\\' -and
        $_.FullName -notmatch '\\\.git\\' -and
        $_.FullName -notmatch '\\\.build\\' -and
        $_.FullName -notmatch '\\build\\' -and
        $_.FullName -notmatch '\\out\\'
    }

if (-not $sources -or $sources.Count -eq 0) {
    throw 'No .cpp files found in workspace.'
}

$sourceArgs = ($sources | ForEach-Object { ('"{0}"' -f $_.FullName) }) -join ' '

$cmdLines = @(
    '@echo off',
    ('call "{0}" -arch=x64 -host_arch=x64' -f $devCmd),
    ('cl /nologo /Zi /EHsc /D NOMINMAX {0} /Fe:"{1}" /Fd:"{2}"' -f $sourceArgs, $exe, $pdb)
)

Set-Content -Path $tmpCmd -Value $cmdLines -Encoding Ascii
try {
    cmd /c $tmpCmd
    exit $LASTEXITCODE
}
finally {
    Remove-Item $tmpCmd -ErrorAction SilentlyContinue
}
