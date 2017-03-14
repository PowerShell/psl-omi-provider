Pester Testing Test Guide for PSRP
==================================

Also see the [Writing Pester Tests](https://github.com/PowerShell/PowerShell/blob/master/docs/testing-guidelines/WritingPesterTests.md)
document.

Test Design Specification for PSRP is here: [TestDesignSpecificationForPSRP]

[TestDesignSpecificationForPSRP]: https://github.com/PowerShell/psl-omi-provider/blob/AutomationTest/test/TestDesign/TestDesignSpecificationForPSRP.md

Install PSRP
--------------------

Run below install script to install PowerShell, OMI and PSRP:

Linux/MacOS
```
chmod +x ./installPSRP.sh
./installPSRP.sh <inputRedmondPassword> <omiversionlike"1.1.0-63"> <psrpversionlike"6">
```

Windows Native PowerShell Command
```
Import-Module ./build.psm1
Start-PSRPInstall - Runtime <runtimeName> -Verbose #<runtimeName> use these: win7-x64,win7-x86,win81-x64,WS2012R2-x64,win10-x64,WS2016-x64
```

Enable NTLM authentication for PSRP Server
--------------------

Run below script to enable NTLM authentication for PSRP Server:

Linux
```
chmod +x ./enableNTLM.sh
./enableNTLM.sh <inputlocalRootPassword>
```

Windows Native PowerShell Command
```
winrm quickconfig
Enable-PSRemoting
winrm set winrm/config/client/auth '@{Basic="true"}'
winrm set winrm/config/client '@{AllowUnencrypted="true"}'
winrm set winrm/config/service/auth '@{Basic="true"}'
winrm set winrm/config/service '@{AllowUnencrypted="true"}'
Set-Item "wsman:\localhost\client\trustedhosts" -Value '*' -Force
```

Note: MacOS currently doesn't support PSRP server.

Running PSRP Pester Tests
--------------------

Go to the top level of the PSRP repository and set environment variables for testing:

(Option 1)Manual setting:

Linux/MacOS
```
export LINUXHOSTNAME="<inputTestLinuxHostName>"
export LINUXUSERNAME="<inputTestLinuxUserName>"
export LINUXPASSWORDSTRING="<inputTestLinuxHostPassword>"
export WINDOWSHOSTNAME="<inputTestWindowsHostName>"
export WINDOWSUSERNAME="<inputTestWindowsUserName>"
export WINDOWSPASSWORDSTRING="<inputTestWindowsHostPassword>"
```

Windows Open PowerShell Command
```
$env:LINUXHOSTNAME="<inputTestLinuxHostName>"
$env:LINUXUSERNAME="<inputTestLinuxUserName>"
$env:LINUXPASSWORDSTRING="<inputTestLinuxHostPassword>"
$env:WINDOWSHOSTNAME="<inputTestWindowsHostName>"
$env:WINDOWSUSERNAME="<inputTestWindowsUserName>"
$env:WINDOWSPASSWORDSTRING="<inputTestWindowsHostPassword>"
```

(Option 2)Script setting:

Linux/MacOS
```
chmod +x setPSRPTest.sh
. setPSRPTest.sh --linux="hostname/username:password" --windows="hostname/username:password"
```

Windows Open PowerShell Command
```
.\setPSRPTest.ps1 -linux "hostname/username:password" -windows "hostname/username:password"
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