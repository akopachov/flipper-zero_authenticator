# Keyboard layouts file format generator script.

param (
    [Parameter(Mandatory)][string]$outputFolder
)

$kbFiles = Invoke-RestMethod -Uri 'https://api.github.com/repos/Flipper-XFW/Xtreme-Firmware/contents/applications/main/bad_kb/resources/badkb/assets/layouts?ref=dev'

foreach ($kbFile in $kbFiles) {
    $layoutsFile = New-Item -Name (Join-Path $outputFolder $kbFile.name) -ItemType File -Force
    $layoutsFileStream = $layoutsFile.OpenWrite()

    $kbFileStream = New-Object System.IO.MemoryStream
    $webStream = [Net.HttpWebRequest]::Create($kbFile.download_url).GetResponse().GetResponseStream()
    try {
        $webStream.CopyTo($kbFileStream)
    }
    finally {
        $webStream.Dispose()
    }
    try {
        Write-Host "Processing $($kbFile.name)"
        Invoke-WebRequest -Uri $kbFile.download_url -OutFile $kbFileStream
        $buffer = New-Object byte[] 72
        $kbFileStream.Position = 96 # Positioning at symbol '0'
        $kbFileStream.Read($buffer, 0, 20) | Out-Null
        Write-Host $buffer

        $kbFileStream.Position = 130 # Positioning at symbol 'A'
        $kbFileStream.Read($buffer, 20, 52) | Out-Null
        $layoutsFileStream.Write($buffer, 0, $buffer.Length)
    }
    finally {
        $kbFileStream.Dispose()
        $layoutsFileStream.Dispose()
    }
}
