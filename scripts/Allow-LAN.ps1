# Optional: run as Administrator if Windows Firewall blocks the device connection.
$ErrorActionPreference='Stop'
$packageRoot=Split-Path $PSScriptRoot -Parent
$config=Get-Content -LiteralPath (Join-Path $env:USERPROFILE '.mosaico-agent-monitor\config.json') -Raw | ConvertFrom-Json
New-NetFirewallRule -DisplayName 'Mosaico Agent Monitor LAN' -Direction Inbound -Action Allow -Protocol TCP -LocalPort $config.port -Program (Join-Path $packageRoot 'runtime\node.exe') -Profile Private -RemoteAddress LocalSubnet | Out-Null
