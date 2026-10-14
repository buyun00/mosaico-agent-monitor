param([Parameter(Mandatory)][string]$Port,[switch]$Check)
$ErrorActionPreference='Stop'
$packageRoot=Split-Path $PSScriptRoot -Parent
$imageRoot=Join-Path $packageRoot 'firmware-images'
$manifest=Get-Content -LiteralPath (Join-Path $imageRoot 'manifest.json') -Raw | ConvertFrom-Json
$flashArguments=@('--chip','esp32s31','--port',$Port,'--no-stub','--before','default-reset','--after','hard-reset','write-flash','--flash-mode','dio','--flash-freq','80m','--flash-size','16MB')
foreach($image in $manifest.images){
  $imagePath=[IO.Path]::GetFullPath((Join-Path $imageRoot $image.file))
  if(-not $imagePath.StartsWith([IO.Path]::GetFullPath($imageRoot)+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Image path is outside the package'}
  if((Get-FileHash -LiteralPath $imagePath -Algorithm SHA256).Hash.ToLowerInvariant() -ne $image.sha256){throw 'Image checksum mismatch'}
  $flashArguments+=@($image.offset,$imagePath)
}
if($Check){Write-Output 'Verified four images. NVS and PHY will be preserved. No device access performed.';return}
$flasher=Join-Path $packageRoot 'runtime\esptool.exe'
& $flasher @flashArguments
if($LASTEXITCODE -ne 0){throw "Firmware flash failed: $LASTEXITCODE"}
