Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Import-Module (Join-Path -Path $PSScriptRoot -ChildPath 'PSRPTests.Common.psm1') -Force

Describe 'Connect with office365' {

    BeforeAll {
        [string] $DescribeError = ''
        $TestVariables = Get-TestVariables([ref] $DescribeError)
        [bool] $Skip = !($TestVariables.IsSupportedEnvironment -and $TestVariables.SupportsOffice365)
    }

    BeforeEach {
        if (!$Skip -and !$DescribeError)
        {
            $session = New-PSSession -Uri $TestVariables.OfficeURI `
                -ConfigurationName $TestVariables.OfficeConfiguration `
                -Credential $TestVariables.OfficeCredential `
                -AllowRedirection `
                -Authentication Basic

            if ($null -eq $session)
            {
                $DescribeError = "Could not create PSSession to $($TestVariables.OfficeURI)"
            }
        }
    }

    AfterEach {
        Get-PSSession | Remove-PSSession
    }

    It 'Verifies New-PSSession connects to Office365 with no errors' -Skip:$Skip {
        $DescribeError | Should -BeNullOrEmpty
        $uri = [Uri]::New($TestVariables.OfficeUri)
        $session.ComputerName | Should -BeExactly $uri.Authority
        $session.ConfigurationName | Should -BeExactly $TestVariables.OfficeConfiguration
    }

    It 'Verifies Invoke-Command succeeds with an Office365 PSSession' -Skip:$Skip {
        $DescribeError | Should -BeNullOrEmpty
        $mailbox = Invoke-Command -Session $session -ScriptBlock {Get-Mailbox -Identity $args[0]} -ArgumentList @($TestVariables.OfficeCredential.UserName)
        # Verify we got back a valid response
        $mailbox | Should -Not -BeNullOrEmpty
        $mailbox.PSComputerName | Should -BeExactly $session.ComputerName
    }
}