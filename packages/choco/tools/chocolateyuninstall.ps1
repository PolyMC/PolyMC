$ErrorActionPreference = 'Stop'; # stop on all errors
$packageArgs = @{
  packageName   = $env:ChocolateyPackageName
  softwareName  = 'PolyMC*'
}

Write-Host "Removing Start Menu and Desktop shortcuts"
$StartMenuShortcut = Join-Path $env:programdata "Microsoft\Windows\Start Menu\Programs\PolyMC.lnk"
$DesktopShortcut = Join-Path $([Environment]::GetFolderPath("Desktop")) "PolyMC.lnk"
if (Test-Path $StartMenuShortcut)
{
  Remove-Item $StartMenuShortcut
}
if (Test-Path $DesktopShortcut)
{
  Remove-Item $DesktopShortcut
}

$polymcdir = Join-Path "$(Get-ToolsLocation)" "PolyMC"
if(Test-Path $polymcdir)
{
  Write-Host "Your instances will be deleted (if they live in the PolyMC directory: $polymcdir)" -ForegroundColor Black -BackgroundColor Yellow
  Write-Host "Ctrl-C to cancel" -ForegroundColor Black -BackgroundColor Yellow
  timeout 10
  Remove-Item -path "$polymcdir" -recurse
}

