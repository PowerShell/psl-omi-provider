<#
    Dependencies

    Environment:
        PSRPHost
        PSRPUserName
        PSRPPassword
#>

function AfterEachCleanup
{
    if ($null -ne $PSSenderInfo)
    {
        Exit-PSSession
    }
    Get-PSSession | Remove-PSSession
}

function SetupTests
{
    if (!(Test-Path -Path env:PSRPHost))
    {
        throw 'Expected env:PSRPHost setting not found'
    }
    if (!(Test-Path -Path env:PSRPUserName))
    {
        throw 'Expected env:PSRPUserName setting not found'
    }
    if (!(Test-Path -Path env:PSRPPassword))
    {
        throw 'Expected env:PSRPPassword setting not found'
    }

    return @{
        HostName = $env:PSRPHost
        Credential = [PSCredential]::new($env:PSRPUserName,  [System.Net.NetworkCredential]::new('', $env:PSRPPassword).SecurePassword)
    }
}

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
    [System.IO.File]::WriteAllBytes($bytes)
}

Describe 'Connect to Windows Server from Linux' {

    BeforeAll {
        $vars = SetupTests
    }

    BeforeEach {
        [System.Management.Automation.Runspaces.PSSession] $session = $null
    }

    AfterEach {
        AfterEachCleanup
    }

    Context 'Connect With -Authentication Negotiate (NTLM)' {

        It 'Verifies New-PSSession -Authentication Negotiate connects with no errors' {
            $session = New-PSSession -ComputerName $vars.Hostname -Credential $vars.Credential -Authentication Negotiate
            $session | Should Not Be $null
        }

        It 'Verifies Enter-PSSession succeeds with a -Authentication Negotiate PSSession -' {
            $session = New-PSSession -ComputerName $vars.Hostname -Credential $vars.Credential -Authentication Negotiate
            $session | Should Not Be $null

            $sessionInfo = Invoke-Command -Session $session -ScriptBlock {Get-Item -Path variable:PSSenderInfo}
            $sessionInfo | Should Not Be $null
        }

        It 'Verifies content can be copied to the target session' {
            $session = New-PSSession -ComputerName $vars.Hostname -Credential $vars.Credential -Authentication Negotiate
            $session | Should Not Be $null
        }
    }

    Context 'Connect with -UseSSL' {

        It 'Verifies New-PSSession -UseSSL connects with no errors' {
            $session = New-PSSession -ComputerName $vars.Hostname -Credential $vars.Credential -Authentication Basic -UseSSL
            $session | Should Not Be $null
        }

        It 'Verifies Invoke-Command succeeds with a -UseSSL PSSession' {
            $session = New-PSSession -ComputerName $vars.Hostname -Credential $vars.Credential -Authentication Basic -UseSSL
            $session | Should Not Be $null

            $sessionInfo = Invoke-Command -Session $session -ScriptBlock {Get-Item -Path variable:PSSenderInfo}
            $sessionInfo | Should Not Be $null
        }
    }

    Context 'Connect with SSH' {
        BeforeAll {

        }
    }
}