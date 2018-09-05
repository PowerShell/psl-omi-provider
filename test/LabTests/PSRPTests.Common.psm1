Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-OSInfo
{
    [CmdletBinding()]
    [OutputType([Hashtable])]
    param
    (
    )

    $osInfo = @{}

    # NOTE: Logic lifted from build.psm1 in the PowerShell repo.
    if ($PSVersionTable.ContainsKey("PSEdition") -and "Core" -eq $PSVersionTable.PSEdition)
    {
        $osInfo.Add('IsCoreCLR', $true)
        $osInfo.Add('IsLinux', $IsLinux)
        $osInfo.Add('IsMacOS', $IsMacOS)
        $osInfo.Add('IsWindows', $IsWindows)
    }
    else
    {
        $osInfo.Add('IsCoreCLR', $false)
        $osInfo.Add('IsLinux', $false)
        $osInfo.Add('IsMacOS', $false)
        $osInfo.Add('IsWindows', $true)
    }

    if ($IsLinux)
    {
        $LinuxInfo = Get-Content /etc/os-release -Raw | ConvertFrom-StringData
        $osInfo.Add('IsDebian', $LinuxInfo.ID -match 'debian')
        $osInfo.Add('IsDebian8', $osInfo.IsDebian -and $LinuxInfo.VERSION_ID -match '8')
        $osInfo.Add('IsDebian9', $osInfo.IsDebian -and $LinuxInfo.VERSION_ID -match '9')
        $osInfo.Add('IsUbuntu', $LinuxInfo.ID -match 'ubuntu')
        $osInfo.Add('IsUbuntu14', $osInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '14.04')
        $osInfo.Add('IsUbuntu16', $osInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '16.04')
        $osInfo.Add('IsUbuntu17', $osInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '17.10')
        $osInfo.Add('IsUbuntu18', $osInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '18.04')
        $osInfo.Add('IsCentOS', $LinuxInfo.ID -match 'centos' -and $LinuxInfo.VERSION_ID -match '7')
        $osInfo.Add('IsFedora', $LinuxInfo.ID -match 'fedora' -and $LinuxInfo.VERSION_ID -ge 24)
        $osInfo.Add('IsOpenSUSE', $LinuxInfo.ID -match 'opensuse')
        $osInfo.Add('IsSLES', $LinuxInfo.ID -match 'sles')
        $osInfo.Add('IsRedHat', $LinuxInfo.ID -match 'rhel')
        $osInfo.Add('IsRedHat7', $osInfo.IsRedHat -and $LinuxInfo.VERSION_ID -match '7')
        $osInfo.Add('IsOpenSUSE13', $osInfo.IsOpenSUSE -and $LinuxInfo.VERSION_ID  -match '13')
        $osInfo.Add('IsOpenSUSE42.1', $osInfo.IsOpenSUSE -and $LinuxInfo.VERSION_ID  -match '42.1')
        $osInfo.Add('IsRedHatFamily', $osInfo.IsCentOS -or $osInfo.IsFedora -or $osInfo.IsRedHat)
        $osInfo.Add('IsSUSEFamily', $osInfo.IsSLES -or $osInfo.IsOpenSUSE)
    }
    return $osInfo
}

$script:TestVariables = @{}

<#
.Synopsis
    Creates a hash table of test variables

.Notes
    The table contains the following items:

    HostName - the value of env:PSRPHost
                Fatal error if not found
    UserName - the value of env:PSRPUserName
                Fatal error if not found
    Password - the value of env:PSRPPassword
               Fatal error if not found
    KeyFilePath - the path to the private key to use for ssh connections
               SSH tests are skipped if not set.
    IsSupportedEnviroment - $true if the current system supports these tests
    SupportsNTLM - $true if the system supports NTLM authentication.
                 - Currently, MacOS does not.
#>
function Get-TestVariables([ref] [string] $errorMessage)
{
    $vars = @{}

    $vars.Add('IsSupportedEnvironment', [bool] $IsLinux)
    $vars.Add('Error', '')

    if (!$IsLinux)
    {
        $errorMessage.Value = 'Unsupported Environment - Linux and MacOS are required'
        $vars.Error = $errorMessage.Value
        return $vars
    }

    $osInfo = Get-OSInfo
    $err = [System.Text.StringBuilder]::new()

    if (!(Test-Path -Path env:PSRPHost))
    {
        $null = $err.AppendLine('$env:PSRPHost was not found')
    }
    else
    {
        $vars.Add('HostName', $env:PSRPHost)
    }

    if (!(Test-Path -Path env:PSRPUserName))
    {
        $null = $err.AppendLine('$env:PSRPUserName was not found')
    }
    else
    {
        $vars.Add('UserName', $env:PSRPUserName)
        if (!(Test-Path -Path env:PSRPPassword))
        {
            $null = $err.AppendLine('$env:PSRPPassword was not found')
        }
        else
        {
            $vars.Add('Credential', [PSCredential]::new($env:PSRPUserName,  [System.Net.NetworkCredential]::new('', $env:PSRPPassword).SecurePassword))
        }
    }

    if (Test-Path -Path env:PSRPKeyFilePath)
    {
        $vars.Add('SupportsSSH', [bool] $true)
        $vars.Add('KeyFilePath', $env:PSRPKeyFilePath)
        if (!(Test-Path -Path $env:PSRPKeyFilePath))
        {
            $null = $err.AppendLine("$env:PSRPKeyFilePath was not found")
        }
    }
    else
    {
        # Must have a private key file to avoid prompting for a password
        $vars.Add('SupportsSSH', [bool] $false)
    }

    #OMI only implemented support for NTLM on Ubuntu and CentOS
    if ($OsInfo.IsUbuntu -or $OsInfo.IsCentOS)
    {
        $vars.Add('SupportsNTLM', $true)
    }
    else
    {
        $vars.Add('SupportsNTLM', $false)
    }

    if ((Test-Path -Path env:OfficeUserName) -and $env:OfficeUserName)
    {
        $vars.Add('OfficeURI', 'https://outlook.office365.com/powershell-liveid')
        $vars.Add('OfficeConfiguration', 'Microsoft.Exchange')
        $vars.Add('OfficeCredential', [PSCredential]::new($env:OfficeUserName, [System.Net.NetworkCredential]::new('', $env:OfficePassword).SecurePassword))
        $vars.Add('SupportsOffice365', $true)
    }
    else
    {
        $null = $err.AppendLine("Could not find env:OfficeUserName")
        $vars.Add('SupportsOffice365', $false)
    }

    if ($err.Length -gt 0)
    {
        $vars.Error = $err.ToString()
        $errorMessage.Value = $vars.Error
    }

    return $vars
}

<#
.Synopsis
    Creates a binary file of the specified size on the test drive.
.Parameter File
    The fully qualified binary file to create.
    If the file already exists, it will be removed.
.Parameter Size
    The size, in bytes, of the file.
.Notes
    Used by Copy-Item tests
#>
function New-TestFile
{
    param
    (
        [string] $File,
        [int] $Size
    )
    if (Test-Path -Path $File)
    {
        Remove-Item -Path $File -Force
    }
    $bytes = [byte[]]::new($Size)
    $random = [System.Random]::new()
    $random.NextBytes($bytes)
    [System.IO.File]::WriteAllBytes($File, $bytes)
}

Export-ModuleMember -Function New-TestFile, Get-TestVariables