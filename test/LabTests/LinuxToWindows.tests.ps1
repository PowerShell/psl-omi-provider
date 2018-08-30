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
    KeyFilePath - the path to the private key to use for ssh connections
               SSH tests are skipped if not set.
    IsSupportedEnviroment - $true if the current system supports these tests
    SupportsNTLM - $true if the system supports NTLM authentication.
                 - Currently, MacOS does not.
#>
function Get-TestVariables([ref] [string] $error)
{
    $vars = @{}
    $err = [System.Text.StringBuilder]::new()

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

        if ($err.Length -gt 0)
        {
            $error.Value = $err.ToString()
            break
        }

        $vars.Add('HostName', $env:PSRPHost)
        $vars.Add('UserName', $env:PSRPUserName)
        $vars.Add('Credential', [PSCredential]::new($env:PSRPUserName,  [System.Net.NetworkCredential]::new('', $env:PSRPPassword).SecurePassword))

        $vars.Add('IsSupportedEnvironment', [bool] $IsLinux)
        $vars.Add('SupportsNTLM', [bool] (!$IsMacOS -and $IsLinux))

    } while ($false)


    return $vars
}

#region Test Implementation

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

        AfterEach {
            Get-PSSession | Remove-PSSession
        }

        It 'Verifies New-PSSession -Authentication Negotiate connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
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

        AfterEach {
            Get-PSSession | Remove-PSSession
        }

        It 'Verifies New-PSSession -UseSSL connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
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
            [string] $ContextError = $DescribeError
            if (!$Skip -and !$ContextError)
            {
                # ISSUE: Only creating the session once for all tests due to
                # recurring connection timeout issues with the target server.
                # Move this to BeforeEach once this is resolved.
                $session = New-PSSession -HostName $TestVariables.Hostname -UserName $TestVariables.UserName -KeyFilePath $TestVariables.KeyFilePath
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSH PSSession to $($TestVariables.HostName)"
                }
            }
        }

        BeforeEach {
        <# Re-enable once connection time issues resolved with the target Windows Server 2016
            [string] $ContextError = $DescribeError
            if (!$Skip -and !$ContextError)
            {
                $session = New-PSSession -HostName $TestVariables.Hostname -UserName $TestVariables.UserName -KeyFilePath $TestVariables.KeyFilePath
                if ($null -eq $session)
                {
                    $ContextError = "Could not create SSH PSSession to $($TestVariables.HostName)"
                }
            }
        #>
        }

        AfterAll {
            Get-PSSession | Remove-PSSession
        }

        It 'Verifies New-PSSession over SSH connects with no errors' -Skip:$Skip {
            $ContextError | Should -BeNullOrEmpty
            $session | Should -Not -Be $null
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