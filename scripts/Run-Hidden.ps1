$ErrorActionPreference='Stop'
$packageRoot=Split-Path $PSScriptRoot -Parent
$stateDir=Join-Path $env:USERPROFILE '.mosaico-agent-monitor'
[IO.Directory]::CreateDirectory($stateDir) | Out-Null
Set-Location -LiteralPath $packageRoot
& (Join-Path $packageRoot 'runtime\node.exe') (Join-Path $packageRoot 'host\server.mjs') 1>> (Join-Path $stateDir 'service.out.log') 2>> (Join-Path $stateDir 'service.err.log')
exit $LASTEXITCODE
