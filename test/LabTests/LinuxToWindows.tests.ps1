Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Import-Module (Join-Path -Path $PSScriptRoot -ChildPath 'PSRPTests.Common.psm1') -Force

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

#region Test Implementation

<#
.Synopsis
    Provides the Copy-Item test over a PSSession.
.Parameter Session
    The PSSession to use as the target of the copy
.Parameter TestVars
    The Test variables shared by all tests.
.Parameter
    The name of the file to create.
.Description
    The test creates a file in the root of the Pester test drive
    then copies it to the home directory on the target system.
    After the copy completes, it verifies the file exists.
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
.Synopsis
    Verifies a session is valid by querying the $PSSenderInfo variable on the target system
    using Invoke-Command
.Parameter session
    The PSSession to the target system.
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

<#
    Provides connectivity tests for PSRP/Linux
    to a target Windows Server using Negotiate, SSL, and OpenSSH
    connections.

    Error reporting:

    Some of the BeforeAll and BeforeEach blocks perform non-trivial
    setup for tests. Typically, when these fail, tests are run but are not
    in a predictable state and failures often do not reflect the root cause.
    To handle this, an error message is created for reporting by the individual
    tests using Should -BeNullOrEmpty.

    For example, if a PSSession cannot be created in BeforeEach, a $ContextError is populated
    in BeforeEach that indicates the session could not be created the target machine. The result
    is BeforeEach error is reported and containing tests report a consistent error.

    If an error occurs in the Describe-level BeforeAll, a DescribeError is populated and subsequent
    Context/It statements use this as the failure text.
#>
Describe 'Connect to Windows Server from Linux' {

    BeforeAll {
        [string] $DescribeError = ''
        $TestVariables = Get-TestVariables([ref] $DescribeError)
    }

    BeforeEach {
        [System.Management.Automation.Runspaces.PSSession] $session = $null
    }

    AfterEach {
        try
        {
            Get-PSSession | Remove-PSSession
        }
        catch
        {
            # ISSUE: ObjectDisposedException: Cannot access a closed file.
        }
    }

    Context 'Connect With -Authentication Negotiate (NTLM)' {

        BeforeAll {
            [bool] $Skip = !($TestVariables.IsSupportedEnvironment -and $TestVariables.SupportsNTLM)
        }

        BeforeEach {
            [string] $ContextError = $DescribeError

            if (!$Skip -and !$ContextError)
            {
                $session = New-PSSession -ComputerName $TestVariables.Hostname -Credential $TestVariables.Credential -Authentication Negotiate
                if ($null -eq $session)
                {
                    $ContextError = "Could not create Negotiate PSSession to $($TestVariables.HostName)"
                }
            }
        }

        It 'Verifies New-PSSession -Authentication Negotiate connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session.ComputerName | Should -BeExactly $TestVariables.Hostname
            $session.Transport | Should -BeExactly 'WSMan'
        }

        It 'Verifies Enter-PSSession -Authentication Negotiate succeeds' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to a Negotiate PSSession' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }

    Context 'Connect with -UseSSL' {

        BeforeAll {
            [bool] $Skip = !$TestVariables.IsSupportedEnvironment
        }

        BeforeEach {
            [string] $ContextError = $DescribeError

            if (!$Skip -and !$ContextError)
            {
                $session = New-PSSession -ComputerName $TestVariables.Hostname -Credential $TestVariables.Credential -Authentication Basic -UseSSL
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSL PSSession to $($TestVariables.HostName)"
                }
            }
        }

        It 'Verifies New-PSSession -UseSSL connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session.ComputerName | Should -BeExactly $TestVariables.Hostname
            $session.Transport | Should -BeExactly 'WSMan'
        }

        It 'Verifies Invoke-Command succeeds with a -UseSSL PSSession' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to a -UseSSL PSSession' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }

    Context 'Connect with SSH' {

        BeforeAll {
            [bool] $Skip = !($TestVariables.IsSupportedEnvironment -and $TestVariables.SupportsSSH)
        }

        BeforeEach {
            [string] $ContextError = $DescribeError
            if (!$Skip -and !$ContextError)
            {
                $session = New-PSSession -HostName $TestVariables.Hostname -UserName $TestVariables.UserName -KeyFilePath $TestVariables.KeyFilePath
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSH PSSession to $($TestVariables.HostName)"
                }
            }
        }

        It 'Verifies New-PSSession over SSH connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session.ComputerName | Should -BeExactly $TestVariables.Hostname
            $session.Transport | Should -BeExactly 'SSH'
        }

        It 'Verifies Invoke-Command succeeds with an SSH PSSession' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-InvokeCommand -Session $session
        }

        It 'Verifies content can be copied to an SSH PSSession'  -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            Test-CopyItem -Session $session -TestFileName 'CopyItemOverNTLM.bin' -TestVars $TestVariables
        }
    }
}