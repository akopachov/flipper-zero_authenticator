param ($enable, $disable)

Push-Location $PSScriptRoot

$featuresConfigContent = Get-Content "totp/features_config.h" -Raw

foreach ($feature in $enable) {
    $featuresConfigContent = $featuresConfigContent -replace "(#undef)(\s+$feature(\s|^)+)", '#define$2'
}

foreach ($feature in $disable) {
    $featuresConfigContent = $featuresConfigContent -replace "(#define)(\s+$feature(\s|^)+)", '#undef$2'
}

Set-Content -Path "totp/features_config.h" -NoNewline -Value $featuresConfigContent

Pop-Location