<#
    Dependencies

    Environment:
        PSRPHost        - The fully qualified host name to connect to.
                        - In the DevTest lab, this should be the internal FQDN
                          or private id.
        PSRPUserName    - A local administrator account on the target system
        PSRPPassword    - The password to use with the local administrator account
        PSRPKeyFilePath - The private key file to use for SSH connections
#>

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
    KeyFileName - the path to the private key to use for ssh connections
               SSH tests are skipped if not set.
#>
function Get-TestVariables([ref] [string] $error)
{
    $vars = @{}

    do
    {
        if (!(Test-Path -Path env:PSRPHost))
        {
            $error.Value = '$env:PSRPHost was not found'
            break
        }

        if (!(Test-Path -Path env:PSRPUserName))
        {
            $error.Value = '$env:PSRPUserName was not found'
            break
        }

        if (!(Test-Path -Path env:PSRPPassword))
        {
            $error.Value = '$env:PSRPPassword was not found'
            break
        }

        $vars.Add('HostName', $env:PSRPHost)
        $vars.Add('UserName', $env:PSRPUserName)
        $vars.Add('Credential', [PSCredential]::new($env:PSRPUserName,  [System.Net.NetworkCredential]::new('', $env:PSRPPassword).SecurePassword))

        if (Test-Path -Path env:PSRPKeyFilePath)
        {
            $vars.Add('KeyFilePath', $env:PSRPKeyFilePath)
            $vars.Add('SkipSSH', $false)
        }
        else
        {
            $vars.Add('SkipSSH', $true)
        }

    } while ($false)


    return $vars
}

#region Test Implementation

<#
    Creates a binary file of the specified size
    on the test drive.
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

<#
    Provides the Copy-Item test over a PSSession.
    Called by tests specific to each connection type.
#>
function Test-CopyItem
{
    param
    (
        [Parameter(Mandatory)]
        [ValidateNotNull()]
        [System.Management.Automation.Runspaces.PSSession] $Session,

        [Parameter(Mandatory)]
        [ValidateNotNull()]
        $TestVars,

        [Parameter(Mandatory)]
        [ValidateNotNullOrEmpty()]
        [string] $TestFileName
    )
    $sourceFile = Join-Path -Path $TestDrive -ChildPath $TestFileName

    # create a binary file
    New-TestFile -File $sourceFile -Size 4096

    # NOTE: Target is a windows system; use windows file path
    $destfile = "c:\users\$($TestVars.UserName)\$testFileName"

    Copy-Item -Path $sourceFile -Destination $destfile -ToSession $Session -ErrorAction Stop -Force
    Invoke-Command -Session $Session -ScriptBlock {Test-Path -Path $args[0]} -Args $destFile | Should -Be $true
}

<#
    Provides the Invoke-Command test over a PSSession
    Called by tests specific to each connection type.
#>
function Test-InvokeCommand
{
    param
    (
        [Parameter(Mandatory)]
        [ValidateNotNull()]
        [System.Management.Automation.Runspaces.PSSession] $session
    )
    $sessionInfo = Invoke-Command -Session $session -ScriptBlock {Get-Item -Path variable:PSSenderInfo}
    $sessionInfo | Should -Not -Be $null
}

#endregion Test Implementation

Describe 'Connect to Windows Server from Linux' {

    BeforeAll {
        [string] $DescribeError = ''
        $TestVariables = Get-TestVariables([ref] $DescribeError)
    }

    BeforeEach {
        [System.Management.Automation.Runspaces.PSSession] $session = $null
    }

    AfterEach {
        Get-PSSession | Remove-PSSession
    }

    Context 'Connect With -Authentication Negotiate (NTLM)' {

        BeforeEach {
            [string] $ContextError = $DescribeError
            if (!$ContextError)
            {
                $session = New-PSSession -ComputerName $TestVariables.Hostname -Credential $TestVariables.Credential -Authentication Negotiate
                if ($null -eq $session)
                {
                    $ContextError = "Could not create Negotiate PSSession to $($TestVariables.HostName)"
                }
            }
        }

        AfterEach {
            $ContextError = ''
        }

        It 'Verifies New-PSSession -Authentication Negotiate connects with no errors' {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
        }

        It 'Verifies Enter-PSSession -Authentication Negotiate succeeds' {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to a Negotiate PSSession' {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }

    Context 'Connect with -UseSSL' {

        BeforeEach {
            [string] $ContextError = $DescribeError
            if (!$ContextError)
            {
                $session = New-PSSession -ComputerName $TestVariables.Hostname -Credential $TestVariables.Credential -Authentication Basic -UseSSL
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSL PSSession to $($TestVariables.HostName)"
                }
            }
        }

        AfterEach {
            $ContextError = ''
        }

        It 'Verifies New-PSSession -UseSSL connects with no errors' {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
        }

        It 'Verifies Invoke-Command succeeds with a -UseSSL PSSession' {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to a -UseSSL PSSession' {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }

    Context 'Connect with SSH' {
        BeforeEach {
            [string] $ContextError = $DescribeError
            if (!$TestVariables.SkipSSH -and !$DescribeError)
            {
                $session = New-PSSession -HostName $TestVariables.Hostname -UserName $TestVariables.UserName -KeyFilePath $TestVariables.KeyFilePath
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSH PSSession to $($TestVariables.HostName)"
                }
            }
        }

        AfterEach {
            $ContextError = ''
        }

        It 'Verifies New-PSSession over SSH connects with no errors' -Skip:$TestVariables.SkipSSH {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
        }

        It 'Verifies Invoke-Command succeeds with an SSH PSSession' -Skip:$TestVariables.SkipSSH {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to an SSH PSSession'  -Skip:$TestVariables.SkipSSH {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }
}