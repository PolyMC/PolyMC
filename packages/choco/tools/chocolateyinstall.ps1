$ErrorActionPreference = 'Stop'; # stop on all errors
$toolsDir   = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
$installDir = Join-Path "$(Get-ToolsLocation)" "PolyMC"
$url        = 'https://github.com/PolyMC/PolyMC/releases/download/1.1.0/PolyMC-Windows-1.1.0.zip'

$packageArgs = @{
  packageName   = $env:ChocolateyPackageName
  unzipLocation = $installDir
  url           = $url
  softwareName  = 'PolyMC*'
  checksum      = 'f6ba30b5de5327ba1e26f06720b87daed146c33a69fd0311c2da1564ee32fbfa'
  checksumType  = 'sha256'
}

Install-ChocolateyZipPackage @packageArgs

Write-Host "Creating Start Menu and Desktop shortcuts"
$startmenu = Join-Path $env:programdata "Microsoft\Windows\Start Menu\Programs"
Install-ChocolateyShortcut -shortcutFilePath $(Join-Path $startmenu "PolyMC.lnk") -TargetPath $(Join-Path $installDir "PolyMC.exe")
$desktop = [Environment]::GetFolderPath("Desktop")
Install-ChocolateyShortcut -shortcutFilePath $(Join-Path $desktop "PolyMC.lnk") -TargetPath $(Join-Path $installDir "PolyMC.exe")
