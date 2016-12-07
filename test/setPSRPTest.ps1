param([String]$linux,[String]$windows)
$linuxarr = $linux -replace "/",":" -split ":"
Write-Host "Setting "$linuxarr[0]
$env:LINUXHOSTNAME=$linuxarr[0]
Write-Host "Setting "$linuxarr[1]
$env:LINUXUSERNAME=$linuxarr[1]
Write-Host "Setting "$linuxarr[2]
$env:LINUXPASSWORDSTRING=$linuxarr[2]

$windowsarr = $windows -replace "/",":" -split ":"
Write-Host "Setting "$windowsarr[0]
$env:WINDOWSHOSTNAME=$windowsarr[0]
Write-Host "Setting "$windowsarr[1]
$env:WINDOWSUSERNAME=$windowsarr[1]
Write-Host "Setting "$windowsarr[2]
$env:WINDOWSPASSWORDSTRING=$windowsarr[2]