Pester Testing Test Guide for PSRP
==================================

Also see the [Writing Pester Tests](https://github.com/PowerShell/PowerShell/blob/master/docs/testing-guidelines/WritingPesterTests.md)
document.

Install PSRP
--------------------

Run below install script to install PowerShell, OMI and PSRP:

Linux/MacOS
```
chmod +x ./installPSRP.sh
./installPSRP.sh
```

Windows Native PowerShell Command
```
Import-Module ./build.psm1
Start-PSRPInstall - Runtime <runtimeName> -Verbose #<runtimeName> use these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64
```

Running PSRP Pester Tests
--------------------

Go to the top level of the PSRP repository and set environment variables for testing:

Linux/MacOS
```
export LINUXHOSTNAME="<inputTestLinuxHostName>"
export LINUXUSERNAME="<inputTestLinuxUserName>"
export WINDOWSHOSTNAME="<inputTestWindowsHostName>"
export WINDOWSUSERNAME="<inputTestWindowsUserName>"
export PASSWORDSTRING="<inputTestHostPassword>"
```

Windows Open PowerShell Command
```
$env:LINUXHOSTNAME="<inputTestLinuxHostName>"
$env:LINUXUSERNAME="<inputTestLinuxUserName>"
$env:WINDOWSHOSTNAME="<inputTestWindowsHostName>"
$env:WINDOWSUSERNAME="<inputTestWindowsUserName>"
$env:PASSWORDSTRING="<inputTestHostPassword>"
```

Then open a self-hosted copy of Open PowerShell.

You should use `Invoke-Pester .\test\PSRP.Tests.ps1` to run the PSRP test on three different platforms to cover all the tests.

Uninstall PSRP
--------------------

Run below uninstall script to uninstall PowerShell, OMI and PSRP:

Linux/MacOS
```
chmod +x ./uninstallPSRP.sh
./uninstallPSRP.sh
```

Windows Native PowerShell Command
```
Import-Module ./build.psm1
Start-PSRPUninstall - Runtime <runtimeName> -Verbose #<runtimeName> use these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64
```