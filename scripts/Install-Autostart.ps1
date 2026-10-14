$ErrorActionPreference='Stop'
$packageRoot=Split-Path $PSScriptRoot -Parent
$shellPath=Join-Path $env:WINDIR 'System32\WindowsPowerShell\v1.0\powershell.exe'
$action=New-ScheduledTaskAction -Execute $shellPath -Argument ('-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File "'+(Join-Path $PSScriptRoot 'Run-Hidden.ps1')+'"') -WorkingDirectory $packageRoot
$principal=New-ScheduledTaskPrincipal -UserId ([Security.Principal.WindowsIdentity]::GetCurrent().Name) -LogonType Interactive -RunLevel Limited
$settings=New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::Zero) -MultipleInstances IgnoreNew -RestartCount 3 -RestartInterval (New-TimeSpan -Minutes 1) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
$hubRoot=Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Codex\StartupHub'
$itemsPath=Join-Path $hubRoot 'items.json'
if(Test-Path -LiteralPath $itemsPath){
  $task=New-ScheduledTask -Action $action -Principal $principal -Settings $settings
  Register-ScheduledTask -TaskName 'Mosaico Agent Monitor' -InputObject $task -Force | Out-Null
  Copy-Item -LiteralPath $itemsPath -Destination ($itemsPath+'.mosaico-backup-'+(Get-Date -Format 'yyyyMMddHHmmss'))
  $items=@(Get-Content -LiteralPath $itemsPath -Raw | ConvertFrom-Json)
  $items=@($items | Where-Object { $_.id -ne 'mosaico-agent-monitor' })
  $items+= [pscustomobject]@{id='mosaico-agent-monitor';name='Mosaico Agent Monitor';purpose='CC/Codex 状态与额度局域网显示';taskPath='\';taskName='Mosaico Agent Monitor';autoStart=$true}
  [IO.File]::WriteAllText($itemsPath,($items | ConvertTo-Json -Depth 12),[Text.UTF8Encoding]::new($false))
  Write-Output 'Added to StartupHub; no separate logon trigger created.'
}else{
  $trigger=New-ScheduledTaskTrigger -AtLogOn -User ([Security.Principal.WindowsIdentity]::GetCurrent().Name)
  Register-ScheduledTask -TaskName 'Mosaico Agent Monitor' -Action $action -Principal $principal -Settings $settings -Trigger $trigger -Force | Out-Null
}
Start-ScheduledTask -TaskName 'Mosaico Agent Monitor'
Write-Output 'Autostart installed. Reboot/logon behavior has not been tested by this command.'
