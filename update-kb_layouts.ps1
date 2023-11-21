# Keyboard layouts file format generator script.

param (
    [string]$outputFile = './totp/assets/kb_layouts.klx',
    [string]$cliHelpFile = './totp/assets/cli/cli_help.txt'
)

function Get-UInt16AsBytes() {
    [OutputType([Array])]
    param (
        [Parameter(Mandatory)][uint16]$Value
    )

    $hi = ($Value -shr 8) -band 0xFF
    $lo = $Value -band 0xFF

    return [Array]@($lo, $hi)
}

function ConvertStringToFixedSizeByteArray {
    param (
        [string]$inputString,
        [int]$fixedSize
    )

    $bytesArray = [System.Text.Encoding]::UTF8.GetBytes($inputString)
    $fixedSizeArray = New-Object byte[] $fixedSize
    [System.Array]::Copy($bytesArray, $fixedSizeArray, [System.Math]::Min($bytesArray.Length, $fixedSizeArray.Length))
    return $fixedSizeArray
}

$kbFiles = Invoke-RestMethod -Uri 'https://api.github.com/repos/Flipper-XFW/Xtreme-Firmware/contents/applications/main/bad_kb/resources/badkb/assets/layouts?ref=dev'

$layoutsFile = New-Item -Name $outputFile -ItemType File -Force
$layoutsFileStream = $layoutsFile.OpenWrite()
$tempFile = New-TemporaryFile
$layouts = @()
try {
    $layoutsFileStream.WriteByte($kbFiles.Length)
    $dataStartOffset = 1 + $kbFiles.Length * 12
    $i = 0
    foreach ($kbFile in $kbFiles) {
        $layoutName = [System.IO.Path]::GetFileNameWithoutExtension($kbFile.name) -replace 'cz_CS', 'cs-CZ'
        $layoutsFileStream.Write((ConvertStringToFixedSizeByteArray -inputString $layoutName -fixedSize 10), 0, 10)
        $layoutsFileStream.Write((Get-UInt16AsBytes -Value ($dataStartOffset + $i * 72)), 0, 2)
        $i = $i + 1
        $layouts += $layoutName
    }
    foreach ($kbFile in $kbFiles) {
        Write-Host "Processing $($kbFile.name)"
        Invoke-WebRequest -Uri $kbFile.download_url -OutFile $tempFile
        try {
            $kbFileStream = $tempFile.OpenRead()
            $buffer = New-Object byte[] 72
            $kbFileStream.Position = 96 # Positioning at symbol '0'
            $kbFileStream.Read($buffer, 0, 20) | Out-Null

            $kbFileStream.Position = 130 # Positioning at symbol 'A'
            $kbFileStream.Read($buffer, 20, 52) | Out-Null
            $layoutsFileStream.Write($buffer, 0, $buffer.Length)
        }
        finally {
            $kbFileStream.Dispose()
        }
    }
}
finally {
    $layoutsFileStream.Dispose()
    Remove-Item $tempFile -Force
}

$cliHelpFileContent = Get-Content -Path $cliHelpFile -Raw
$cliHelpFileContent = $cliHelpFileContent -replace '(?m)(-k <layout>.+Must be one of: )(.+)$', ('$1' + ($layouts -join ', '))
Set-Content -Path $cliHelpFile -Value $cliHelpFileContent -NoNewline -Force -Encoding Ascii | Out-Null
