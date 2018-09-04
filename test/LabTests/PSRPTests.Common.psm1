Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$script:OsInfo = @{}

function Get-OSInfo
{
    [CmdletBinding()]
    [OutputType([Hashtable])]
    param
    (
    )

    if ($script:OsInfo.ContainsKey('IsLinux'))
    {
        return $script:OsInfo
    }

    # NOTE: Logic lifted from build.psm1 in the PowerShell repo.
    if ($PSVersionTable.ContainsKey("PSEdition") -and "Core" -eq $PSVersionTable.PSEdition)
    {
        $script:OsInfo += @{'IsCoreCLR' = $true}
        $script:OsInfo += @{'IsLinux' = $IsLinux}
        $script:OsInfo += @{'IsMacOS' = $IsMacOS}
        $script:OsInfo += @{'IsWindows' = $IsWindows}
    }
    else
    {
        $script:OsInfo += @{'IsCoreCLR' = $false}
        $script:OsInfo += @{'IsLinux' = $false}
        $script:OsInfo += @{'IsMacOS' = $false}
        $script:OsInfo += @{'IsWindows' = $true}
    }

    if ($IsLinux)
    {
        $LinuxInfo = Get-Content /etc/os-release -Raw | ConvertFrom-StringData
        $script:OsInfo += @{'IsDebian' = $LinuxInfo.ID -match 'debian'}
        $script:OsInfo += @{'IsDebian8' = $script:OsInfo.IsDebian -and $LinuxInfo.VERSION_ID -match '8'}
        $script:OsInfo += @{'IsDebian9' = $script:OsInfo.IsDebian -and $LinuxInfo.VERSION_ID -match '9'}
        $script:OsInfo += @{'IsUbuntu' = $LinuxInfo.ID -match 'ubuntu'}
        $script:OsInfo += @{'IsUbuntu14' = $script:OsInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '14.04'}
        $script:OsInfo += @{'IsUbuntu16' = $script:OsInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '16.04'}
        $script:OsInfo += @{'IsUbuntu17' = $script:OsInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '17.10'}
        $script:OsInfo += @{'IsUbuntu18' = $script:OsInfo.IsUbuntu -and $LinuxInfo.VERSION_ID -match '18.04'}
        $script:OsInfo += @{'IsCentOS' = $LinuxInfo.ID -match 'centos' -and $LinuxInfo.VERSION_ID -match '7'}
        $script:OsInfo += @{'IsFedora' = $LinuxInfo.ID -match 'fedora' -and $LinuxInfo.VERSION_ID -ge 24}
        $script:OsInfo += @{'IsOpenSUSE' = $LinuxInfo.ID -match 'opensuse'}
        $script:OsInfo += @{'IsSLES' = $LinuxInfo.ID -match 'sles'}
        $script:OsInfo += @{'IsRedHat' = $LinuxInfo.ID -match 'rhel'}
        $script:OsInfo += @{'IsRedHat7' = $script:OsInfo.IsRedHat -and $LinuxInfo.VERSION_ID -match '7' }
        $script:OsInfo += @{'IsOpenSUSE13' = $script:OsInfo.IsOpenSUSE -and $LinuxInfo.VERSION_ID  -match '13'}
        $script:OsInfo += @{'IsOpenSUSE42.1' = $script:OsInfo.IsOpenSUSE -and $LinuxInfo.VERSION_ID  -match '42.1'}
        $script:OsInfo += @{'IsRedHatFamily' = $script:OsInfo.IsCentOS -or $script:OsInfo.IsFedora -or $script:OsInfo.IsRedHat}
        $script:OsInfo += @{'IsSUSEFamily' = $script:OsInfo.IsSLES -or $script:OsInfo.IsOpenSUSE}
    }

    return $script:OsInfo
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
    $vars = $script:TestVariables

    # Determine if we have already initialized.
    if ($vars.ContainsKey('IsSupportedEnvironment'))
    {
        $errorMessage.Value = $vars.Error
        return $vars
    }

    $vars.Add('IsSupportedEnvironment', [bool] $IsLinux)
    $vars.Add('Error', '')

    if (!$IsLinux)
    {
        $errorMessage.Value = 'Unsupported Environment - Linux and MacOS are required'
        $vars.Error = $errorMessage.Value
        return $vars
    }
    $vars.IsSupportedEnvironment = $true

    $err = [System.Text.StringBuilder]::new()

    $osInfo = Get-OSInfo

    do
    {
        if (!(Test-Path -Path env:PSRPHost))
        {
            $null = $err.AppendLine('$env:PSRPHost was not found')
        }

        if (!(Test-Path -Path env:PSRPUserName))
        {
            $null = $err.AppendLine('$env:PSRPUserName was not found')
        }

        if (!(Test-Path -Path env:PSRPPassword))
        {
            $null = $err.AppendLine('$env:PSRPPassword was not found')
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
        if ($script:OsInfo.IsUbuntu -or $script:OsInfo.IsCentOS)
        {
            $vars.Add('SupportsNTLM', $true)
        }
        else
        {
            $vars.Add('SupportsNTLM', $false)
        }

        if (Test-Path -Path env:OfficeUserName)
        {
            $vars.Add('OfficeURI', 'https://outlook.office365.com/powershell-liveid')
            $vars.Add('OfficeConfiguration', 'Microsoft.Exchange')
            $vars.Add('OfficeCredential', [PSCredential]::new($env:OfficeUserName, [System.Net.NetworkCredential]::new('', $env:OfficePassword).SecurePassword))
            $vars.Add('SupportsOffice365', $true)
        }
        else
        {
            $vars.Add('SupportsOffice365', $false)
        }

        $vars.Add('HostName', $env:PSRPHost)
        $vars.Add('UserName', $env:PSRPUserName)
        $vars.Add('Credential', [PSCredential]::new($env:PSRPUserName,  [System.Net.NetworkCredential]::new('', $env:PSRPPassword).SecurePassword))

    } while ($false)

    if ($err.Length -gt 0)
    {
        $vars.Error = $err.ToString()
        $errorMessage.Value = $err.ToString()
        break
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