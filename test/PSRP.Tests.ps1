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
            $badHostName = "badHostName"
            $Port = 5986
        }

        #PowerShell from Windows/Linux/MacOS to Linux with basic authentication over https should work
        It "001:<Basic><HTTPS><Windows/Linux/Mac-Linux>:Powershell with valid credentials should work." {
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

        #PowerShell from Windows to Linux with basic authentication with bad username over https should throw exception
        It "002:<Basic><HTTPS><Windows-Linux>: Powershell with invalid username should throw exception." -Skip:($IsLinux -Or $IsOSX) {
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
        
        #PowerShell from Windows to Linux with basic authentication with bad password over https should throw exception
        It "003:<Basic><HTTPS><Windows-Linux>: Powershell with invalid password should throw exception." -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $badPassword -AsPlainText -Force
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

        #PowerShell from Linux/MacOS to Linux with basic authentication with bad username over https should throw exception
        It "004:<Basic><HTTPS><Linux/Mac-Linux>: Powershell with invalid username should throw exception." -Skip:$IsWindows {
            $hostname = $LinuxHostName
            $User = $badUserName
            $password=$linuxPasswordString
            $PWord = Convertto-SecureString $password -AsPlainText -Force
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

        #PowerShell from Linux/MacOS to Linux with basic authentication with bad password over https should throw exception
        It "005:<Basic><HTTPS><Linux/Mac-Linux>: Powershell with invalid password should throw exception." -Skip:$IsWindows {
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

      
        #Powershell from Windows to Linux with basic authentication with invalid hostname over https should throw exception
        It "006:<Basic><HTTPS><Windows-Linux>: Powershell with invalid hostname should throw exception." -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $badHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "ComputerNotFound,PSSessionOpenFailed"
            }
        }

        #Powershell from Linux/MacOS to Linux with basic authentication with omicli with bad password over https should throw exception
        It "007:<Basic><HTTPS><Linux/Mac-Linux>: Powershell with invalid hostname should throw exception." -Skip:$IsWindows {
            $hostname = $badHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }

        #Skip Windows to Windows because of not support.
        #PowerShell from Linux/MacOS to Windows with basic authentication over http should work
        It "008:<Basic><HTTP><Linux/Mac-Windows>: Powershell with valid credentials should work." -Skip:($IsWindows) {
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

        #Powershell from Liunx/MacOS to Windows with basic authentication over https over http with bad username should throw exception
        It "009:<Basic><HTTP><Linux/Mac-Windows>: Powershell with invalid username should throw exception." -Skip:($IsWindows) {
            $hostname = $WindowsHostName
            $User = $badUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }

        #Powershell from Liunx/MacOS to Windows with basic authentication over https over http with bad password should throw exception
        It "010:<Basic><HTTP><Linux/Mac-Windows>: Powershell with invalid password should throw exception." -Skip:($IsWindows) {
            $hostname = $WindowsHostName
            $User = $WindowsUserName
            $password=$badPassword
            $PWord = Convertto-SecureString $password -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }
            
        #Powershell from Liunx/MacOS to Windows with basic authentication over https over http with bad hostname should throw exception
        It "011:<Basic><HTTP><Linux/Mac-Windows>: Powershell with invalid hostname should throw exception." -Skip:($IsWindows) {
            $hostname = $badHostName
            $User = $WindowsUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }
        
        #PowerShell from Windows/Linux to Linux with negotiate authentication over https should work
        It "012:<Negotiate><HTTPS><Windows/Linux-Linux>: Powershell with valid credentials should work." -Skip:($IsOSX)  {
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

        #PowerShell from Linux/MacOS to Linux with negotiate authentication with bad username over https should throw exception
        It "013:<Negotiate><HTTPS><Linux-Linux>: Powershell with invalid username should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                # it maybe a error message issue on Linux/MacOS, just keep it now.
                $_.FullyQualifiedErrorId | Should be "2,PSSessionOpenFailed"
            }
        }
        

        #PowerShell from Linux/MacOS to Linux with negotiate authentication with bad password over https should throw exception
        It "014:<Negotiate><HTTPS><Linux-Linux>: Powershell with invalid password should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $badPassword -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                # it maybe a error message issue on Linux/MacOS, just keep it now.
                $_.FullyQualifiedErrorId | Should be "2,PSSessionOpenFailed"
            }
        }

        #PowerShell from Windows to Linux with negotiate authentication with bad username over https should throw exception
        It "015:<Negotiate><HTTPS><Windows-Linux>: Powershell with invalid username should throw exception." -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $badUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
        }
   
        #PowerShell from Windows to Linux with negotiate authentication with bad password over https should throw exception
        It "016:<Negotiate><HTTPS><Windows-Linux>: Powershell with invalid password should throw exception." -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $LinuxHostName
            $User = $LinuxUserName
            $password=$badPassword
            $PWord = Convertto-SecureString $password -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "AccessDenied,PSSessionOpenFailed"
            }
        }

        #PowerShell from Windows to Linux with negotiate authentication with bad hostname over https should throw exception
        It "017:<Negotiate><HTTPS><Windows-Linux>: Powershell with invalid hostname should throw exception." -Skip:($IsLinux -Or $IsOSX) {
            $hostname = $badHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User,$PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                 $_.FullyQualifiedErrorId | Should be "ComputerNotFound,PSSessionOpenFailed"
            }
        }

        #PowerShell from Linux/MacOS to Linux with negotiate authentication with bad hostname over https should throw exception
        It "018:<Negotiate><HTTPS><Linux-Linux>: Powershell with invalid hostname should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $badHostName
            $User = $LinuxUserName
            $PWord = Convertto-SecureString $linuxPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                # it maybe a error message issue on Linux/MacOS, just keep it now.
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }

        #Skip from Windows to Windows scenario because this is not supported now.
        #PowerShell from Linux to Windows with negotiate authentication over http should work
        It "019:<Negotiate><HTTP><Linux-Windows>: Powershell with valid credentials should work." -Skip:($IsWindows -Or $IsOSX) {
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

        #Powershell from Liunx to Windows with negotiate authentication over https over http with bad username should throw exception
        It "020:<Negotiate><HTTP><Linux-Windows>: Powershell with invalid username should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $WindowsHostName
            $User = $badUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication negotiate -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }

        #Powershell from Liunx to Windows with negotiate authentication over https over http with bad password should throw exception
        It "021:<Negotiate><HTTP><Linux-Windows>: Powershell with invalid password should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $WindowsHostName
            $User = $WindowsUserName
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
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }

        #Powershell from Liunx to Windows with negotiate authentication over https over http with bad hostname should throw exception
        It "022:<Negotiate><HTTP><Linux-Windows>: Powershell with invalid hostname should throw exception." -Skip:($IsWindows -Or $IsOSX) {
            $hostname = $badHostName
            $User = $WindowsUserName
            $PWord = Convertto-SecureString $windowsPasswordString -AsPlainText -Force
            $cred = New-Object -TypeName System.Management.Automation.PSCredential -ArgumentList $User, $PWord
            $sessionOption = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
            try
            {
                $mySession = New-PSSession -ComputerName $hostname -Credential $cred -Authentication Basic -UseSSL -SessionOption $sessionOption -ErrorAction Stop
                Invoke-Command -Session $mySession {Get-Host}
            }
            catch
            {
                $_.FullyQualifiedErrorId | Should be "1,PSSessionOpenFailed"
            }
        }
}