<#
    .Synopsis
       Download the prerequisites needed to build Kodi

    .DESCRIPTION
        Downloads pre-built dependencies for the specified
        platform and architecture.

    .PARAMETER Arch
       The architecture to download dependencies for x86/x64/arm

    .PARAMETER Platform
        The platform to download dependencies for desktop/uwp
#>
param (
    [parameter(Position=0)]
    [ValidateSet('x86','x64','arm')]
    [string]$Arch = 'x86',

    [Parameter(Position=1)]
    [ValidateSet('desktop', 'uwp')]
    [string] $Platform = 'desktop',

    [Parameter(Position=2)]
    [string] $Mirror = 'http://mirrors.kodi.tv/'
)

$basePath = Resolve-Path (Join-Path (Split-Path -Path $PSScriptRoot -Parent) "..\..")
$dependencyPath = Join-Path $basePath "project\BuildDependencies\$Arch-$Platform"
$downloadFolder = Join-Path $basePath "project\BuildDependencies\downloads"
$sourcesPath = Join-Path $basePath "tools\buildsteps\windows\dependencies"
Write-Verbose $basePath
Write-Verbose $dependencyPath
Write-Verbose $downloadFolder

$nativeDeps = @()
$nativePath = Join-Path $sourcesPath "$Arch-$Platform-native.txt"
if (Test-Path $nativePath) {
    $nativeDeps = Get-Content $nativePath
} else {
    $nativeDeps = Get-Content (Join-Path $sourcesPath "x86-desktop-native.txt")
}

$targetDeps = Get-Content (Join-Path $sourcesPath "$Arch-$Platform-target.txt")

$deps = $nativeDeps + $targetDeps
# Start working
foreach ($dependency in $deps.Where{-not $_.StartsWith(";")}) {
    $destination = Join-Path $downloadFolder $dependency
    $url = "$Mirror/build-deps/win32/$dependency"
        
    if (Test-Path $destination) {
        Write-Verbose "Already downloaded, skipping"
        continue
    }
    Invoke-WebRequest -Uri $url -OutFile $destination
    Unblock-File $destination
    # Expand-Archive $destination -DestinationPath $dependencyPath
}