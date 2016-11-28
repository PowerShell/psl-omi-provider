Describe " PowerShell Remoting basic functional tests" -Tag @("CI") {
        BeforeALL {
                    $LinuxHostName = $env:LINUXHOSTNAME
                    $LinuxUserName = $env:LINUXUSERNAME
                    $WindowsHostName = $env:WINDOWSHOSTNAME
                    $WindowsUserName = $env:WINDOWSUSERNAME
                    $passwordString = $env:PASSWORDSTRING
                    $badUserName = "badUserName"
                    $badPassword = "badPassword"
                    $Port = 5986
        }

        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication should work" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = convertto-securestring $passwordString -asplaintext -force
            $cred = New-Object –TypeName System.Management.Automation.PSCredential –ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result|Should Not BeNullOrEmpty
        }
        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with winrm/omicli should work" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u $User -p $passwordString --auth Basic --hostname $hostname --port $Port --encryption https
                $result|Should Not BeNullOrEmpty
            }
            elseif($IsWindows)
            {
                $result = winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$passwordString -skipcncheck -skipcacheck -encoding:utf-8
                $result|Should Not BeNullOrEmpt
            }
        }
        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with bad username should throw exception" {
            $hostname = $LinuxHostName
            $User = $badUserName
            $PWord = convertto-securestring $passwordString -asplaintext -force
            $cred = New-Object –TypeName System.Management.Automation.PSCredential –ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
            
        }
        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with bad password should throw exception" {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = convertto-securestring $badPassword -asplaintext -force
            $cred = New-Object –TypeName System.Management.Automation.PSCredential –ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
            
        }
        #omicli pending when using bad username
        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with winrm/omicli with bad username should throw exception" -Pending:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName

            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u $User -p $passwordString --auth Basic --hostname $hostname --port $Port --encryption https
                $result | Should Match 'error'
           }
           elseif($IsWindows)
           {
                try
                {
                    winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$passwordString -skipcncheck -skipcacheck -encoding:utf-8
                }
                catch
                {
                    $_.FullyQualifiedErrorId | Should be "NativeCommandError"
                }
           }
        }
        #omicli pending when using bad password
        It "Remoting from Windows/Linux/MacOS to Linux with basic authentication with winrm/omicli with bad password should throw exception" -Pending:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            
            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u $User -p $badPassword --auth Basic --hostname $hostname --port $Port --encryption https
                $result | Should Match 'error'
           }
           elseif($IsWindows)
           {
                try
                {
                    winrm enumerate http://schemas.microsoft.com/wbem/wscim/1/cim-schema/2/OMI_Identify?__cimnamespace=root/omi -r:https://${hostname}:$Port -auth:Basic -u:$User -p:$badPassword -skipcncheck -skipcacheck -encoding:utf-8
                }
                catch
                {
                    $_.FullyQualifiedErrorId | Should be "NativeCommandError"
                }
           }
            
        }
        #Skip Windows to Windows because of not support.
        It "Remoting from Linux/MacOS to Windows with basic authentication should work" -Skip:($IsWindows){
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            $PWord = convertto-securestring $passwordString -asplaintext -force
            $cred = New-Object –TypeName System.Management.Automation.PSCredential –ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption
            $result = Invoke-Command -Session $mySession {Get-Host}
            $result|Should Not BeNullOrEmpty
        }
        #Skip Windows to Windows because of not support.
        It "Remoting from Linux/MacOS to Windows with basic authentication by omicli should work" -Skip:($IsWindows){
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            if($IsLinux -Or $IsOSX)
            {
                $result = /opt/omi/bin/omicli id -u $User -p $passwordString --auth Basic --hostname $hostname --port $Port --encryption https
                $result|Should Not BeNullOrEmpty
            }
        }
    }