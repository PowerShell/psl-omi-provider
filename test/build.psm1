#Basic variables initial
$DownloadFolder = "https://github.com/PowerShell/PowerShell/releases/download/"
$ReleaseTag = "v6.0.0-alpha.13"
$Endfix = $ReleaseTag.Substring($ReleaseTag.LastIndexOf(".")+1,$ReleaseTag.Length - $ReleaseTag.LastIndexOf(".")-1)
$MSIversion = $ReleaseTag.Substring(1,$ReleaseTag.Length -1 ).Replace("-",".$Endfix-")
$version = $MSIversion.Substring(0,$MSIversion.IndexOf("-"))
$ReleaseFolder = $DownloadFolder + $ReleaseTag
$powershell = "C:\Program Files\PowerShell\$version\powershell.exe"

function Start-PSRPInstall {
    [CmdletBinding(DefaultParameterSetName='CoreCLR')]
    param(
        [ValidateSet("win7-x64",
                     "win7-x86",
                     "win81-x64",
                     "win10-x64")]
        [Parameter(ParameterSetName='CoreCLR')]
        [string]$Runtime
    )
    if($Runtime -ne $null -and ($Runtime -ne ""))
    {
        if($Runtime -eq "win7-x64")
        {
            $msiFile = "PowerShell_$MSIversion-win7-x64.msi"
            $msiURLFile = $ReleaseFolder+"/"+$msiFile
        }
        elseif($Runtime -eq "win7-x86")
        {
            $msiFile = "PowerShell_$MSIversion-win7-x86.msi"
            $msiURLFile = $ReleaseFolder+"/"+$msiFile
        }
        elseif($Runtime -eq "win81-x64" -or ($Runtime -eq "WS2012R2-x64"))
        {
            $msiFile = "PowerShell_$MSIversion-win81-x64.msi"
            $msiURLFile = $ReleaseFolder+"/"+$msiFile
        }
        elseif($Runtime -eq "win10-x64" -or ($Runtime -eq "WS2016-x64"))
        {
            $msiFile = "PowerShell_$MSIversion-win10-x64.msi"
            $msiURLFile = $ReleaseFolder+"/"+$msiFile
        }
        else
        {
            Write-Error "$Runtime is not support Runtime! Please use these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64!"
        }

        Write-Verbose "Downloading PowerShell packages from $msiURLFile"
        Invoke-WebRequest -Uri $msiURLFile -OutFile $PSScriptRoot\$msiFile
        Write-Verbose "Installing packages: $PSScriptRoot\$msiFile"
        msiexec /i $PSScriptRoot\$msiFile /qn
        Write-Verbose "[Done]:Installed packages: $PSScriptRoot\$msiFile"
    }
    else
    {
        Write-Error "Runtime should not be empty! Please use -Runtime argument and value as these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64!"
    }
 }

 function Start-PSRPUninstall {
    [CmdletBinding(DefaultParameterSetName='CoreCLR')]
    param(
        [ValidateSet("win7-x64",
                     "win7-x86",
                     "win81-x64",
                     "win10-x64",
                     "osx.10.11-x64")]
        [Parameter(ParameterSetName='CoreCLR')]
        [string]$Runtime
    )
    if($Runtime -ne $null -and ($Runtime -ne ""))
    {
        if($Runtime -eq "win7-x64")
        {
            $msiFile = "PowerShell_$MSIversion-win7-x64.msi"
        }
        elseif($Runtime -eq "win7-x86")
        {
            $msiFile = "PowerShell_$MSIversion-win7-x86.msi"
        }
        elseif($Runtime -eq "win81-x64" -or ($Runtime -eq "WS2012R2-x64"))
        {
            $msiFile = "PowerShell_$MSIversion-win81-x64.msi"
        }
        elseif($Runtime -eq "win10-x64" -or ($Runtime -eq "WS2016-x64"))
        {
            $msiFile = "PowerShell_$MSIversion-win10-x64.msi"
        }
        else
        {
            Write-Error "$Runtime is not support Runtime! Please use these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64!"
        }
        Write-Verbose "Uninstalling packages: $PSScriptRoot\$msiFile"
        msiexec /x $PSScriptRoot\$msiFile /qn
        Write-Verbose "[Done]:Uninstalled packages: $PSScriptRoot\$msiFile"
    }
    else
    {
        if($msiFile -ne $null -and ($msiFile -ne ""))
        {
            Write-Verbose "Uninstalling packages: $PSScriptRoot\$msiFile"
            msiexec /x $PSScriptRoot\$msiFile /qn
            Write-Verbose "[Done]:Uninstalled packages: $PSScriptRoot\$msiFile"
        }
        else
        {
            Write-Error "Runtime should not be empty! Please use -Runtime argument and value as these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64!"
        }
    }
 }


function Start-PSRPPester {
    [CmdletBinding()]
    param(
        [string]$OutputFormat = "NUnitXml",
        [string]$OutputFile = "pester-tests.xml",
        [string]$Path = "$PSScriptRoot/test"
    )

    Write-Verbose "Running pester tests at '$path'" -Verbose

    # All concatenated commands/arguments are suffixed with the delimiter (space)
    $Command = ""

    # Windows needs the execution policy adjusted
    if ($IsWindows) {
        $Command += "Set-ExecutionPolicy -Scope Process Unrestricted; "
    }
    
    $Command += "Invoke-Pester "

    $Command += "-OutputFormat ${OutputFormat} -OutputFile ${OutputFile} "

    $Command += "'" + $Path + "/PSRP.Tests.ps1'"

    Write-Verbose $Command

    & $powershell -noprofile -c $Command
}