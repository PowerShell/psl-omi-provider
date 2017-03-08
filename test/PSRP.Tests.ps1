Describe " PowerShell Remoting basic functional tests" -Tag @("CI") {
        BeforeAll {
            $LinuxHostName = $env:LINUXHOSTNAME
            $LinuxUserName = $env:LINUXUSERNAME
            $linuxPasswordString = $env:LINUXPASSWORDSTRING
            $WindowsHostName = $env:WINDOWSHOSTNAME
            $WindowsUserName = $env:WINDOWSUSERNAME
            $windowsPasswordString = $env:WINDOWSPASSWORDSTRING
            $badUserName = "badUserName"
            $badPassword = "badPassword"
            $Port = 5986
        }

        It "PowerShell from Windows/Linux/MacOS to Linux with basic authentication over https should work" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $password=$linuxPasswordString
            $PWord = convertto-securestring $password -asplaintext -force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result.PSComputerName|Should Not BeNullOrEmpty
            # Linux/MacOS to Linux: Disconnect-PSSession not works, error:"To support disconnecting, the remote computer must be running Windows PowerShell 3.0 or a later version of Windows PowerShell.", just skip it.
            if($IsWindows)
            {
                Get-PSSession|Disconnect-PSSession
            }
            Get-PSSession|Remove-PSSession
        }

        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with omicli over https should work" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u $User -p $linuxPasswordString --auth Basic --hostname $hostname --port $Port --encryption https
                $result|Should Not BeNullOrEmpty
            }
        }

        It "WSMan from Windows to Linux with basic authentication with winrm over https should work"  -Skip:($IsLinux -Or $IsOSX){
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $result = winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$linuxPasswordString -skipcncheck -skipcacheck -encoding:utf-8
            # "$result | Should Not BeNullOrEmpty" not works here, it is a Pester bug, so just use Length verificaiton now
            $result.Length|Should BeGreaterThan 1
        }

        It "PowerShell from Windows/Linux/MacOS to Linux with basic authentication with bad username over https should throw exception" -Skip:$IsWindows {
            $hostname = $LinuxHostName
            $User = $badUserName
            $password=$linuxPasswordString
            $PWord = Convertto-SecureString $password -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                # it maybe a error message issue on Linux/MacOS, just keep it now.
                $_.FullyQualifiedErrorId | Should be "2,PSSessionOpenFailed"
            }
        }

        It "PowerShell from Windows/Linux/MacOS to Linux with basic authentication with bad username over https should throw exception" -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName
            $password=$linuxPasswordString
            $PWord = Convertto-SecureString $password -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
        }

        It "PowerShell from Linux/MacOS to Linux with basic authentication with bad password over https should throw exception" -Skip:$IsWindows {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $badPassword -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                # it maybe a error message issue on Linux/MacOS, just keep it now.
                $_.FullyQualifiedErrorId | Should be "2,PSSessionOpenFailed"
            }
        }

        It "PowerShell from Windows to Linux with basic authentication with bad password over https should throw exception" -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $badPassword -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
        }

        #omicli pending when using bad username
        It "Remoting from Linux/MacOS to Linux with basic authentication with omicli with bad username over https should throw exception" -Skip:($IsWindows) -Pending:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName

            $result = /opt/omi/bin/omicli id -u $User -p $linuxPasswordString --auth Basic --hostname $hostname --port $Port --encryption https
            $result | Should Match 'error'
        }

        It "WSMan from Windows to Linux with basic authentication with winrm with bad username over https should throw exception" -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName

            try
            {
                 winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$linuxPasswordString -skipcncheck -skipcacheck -encoding:utf-8
            }
            catch
            {
                 $_.FullyQualifiedErrorId | Should be "NativeCommandError"
            }
        }

        #omicli pending when using bad password
        It "Remoting from Linux/MacOS to Linux with basic authentication with omicli with bad password over https should throw exception" -Skip:($IsWindows) -Pending:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $result = /opt/omi/bin/omicli id -u $User -p $badPassword --auth Basic --hostname $hostname --port $Port --encryption https
            $result | Should Match 'error'
        }

        It "WSMan from Windows to Linux with basic authentication with winrm with bad password over https should throw exception" -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            
            try
            {
                winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$badPassword -skipcncheck -skipcacheck -encoding:utf-8
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "NativeCommandError"
            }
        }

        #Skip Windows to Windows because of not support.
        It "Remoting from Linux/MacOS to Windows with basic authentication over http should work" -Skip:($IsWindows) {
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result|Should Not BeNullOrEmpty
            # Linux/MacOS to Windows: Disconnect-PSSession not works, error:"To support disconnecting, the remote computer must be running Windows PowerShell 3.0 or a later version of Windows PowerShell.", just skip it.
            if($IsWindows)
            {
                Get-PSSession|Disconnect-PSSession
            }
            Get-PSSession|Remove-PSSession
        }

        It "Remoting from Linux/MacOS to Windows with basic authentication by omicli over http should work" -Skip:($IsWindows) {
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            $HTTPPort=5985
            $result = /opt/omi/bin/omicli ei root/cimv2 Win32_SystemOperatingSystem -u $User -p $windowsPasswordString --auth Basic --hostname $hostname --port $HTTPPort --encryption none
            $result|Should Not BeNullOrEmpty
        }

        It "PowerShell from Windows/Linux/MacOS to Linux with negotiate authentication over https should work" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $password=$linuxPasswordString
            $PWord = convertto-securestring $password -asplaintext -force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList "$hostname\$User",$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result.PSComputerName|Should Not BeNullOrEmpty
            # Linux/MacOS to Linux: Disconnect-PSSession not works, error:"To support disconnecting, the remote computer must be running Windows PowerShell 3.0 or a later version of Windows PowerShell.", just skip it.
            if($IsWindows)
            {
                Get-PSSession|Disconnect-PSSession
            }
            Get-PSSession|Remove-PSSession
        }

        It "Remoting from Linux/MacOS to Linux with negotiate authentication with omicli over https should work" -SKip:($IsWindows) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u "$hostname\$User" -p $linuxPasswordString --auth NegoWithCreds --hostname $hostname --port $Port --encryption https
                $result|Should Not BeNullOrEmpty
            }
        }

        It "WSMan from Windows to Linux with negotiate authentication with winrm over https should work" -SKip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $result = winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:negotiate -u:"$hostname\$User" -p:$linuxPasswordString -skipcncheck -skipcacheck -encoding:utf-8
            # "$result | Should Not BeNullOrEmpty" not works here, it is a Pester bug, so just use Length verificaiton now
            $result.Length|Should BeGreaterThan 1
        }

        #Skip Windows to Windows because of not support.
        It "PowerShell from Linux/MacOS to Windows with negotiate authentication over http should work" -Skip:($IsWindows) {
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList "$hostname\$User", $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result|Should Not BeNullOrEmpty
            # Linux/MacOS to Windows: Disconnect-PSSession not works, error:"To support disconnecting, the remote computer must be running Windows PowerShell 3.0 or a later version of Windows PowerShell.", just skip it.
            if($IsWindows)
            {
                Get-PSSession|Disconnect-PSSession
            }
            Get-PSSession|Remove-PSSession
        }
    }