Pester Testing Test Guide for PSRP
==================================

Also see the [Writing Pester Tests](https://github.com/PowerShell/PowerShell/blob/master/docs/testing-guidelines/WritingPesterTests.md)
document.

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

Windows Command Line
```
set LINUXHOSTNAME="<inputTestLinuxHostName>"
set LINUXUSERNAME="<inputTestLinuxUserName>"
set WINDOWSHOSTNAME="<inputTestWindowsHostName>"
set WINDOWSUSERNAME="<inputTestWindowsUserName>"
set PASSWORDSTRING="<inputTestHostPassword>"
```

Windows PowerShell Command
```
$env:LINUXHOSTNAME="<inputTestLinuxHostName>"
$env:LINUXUSERNAME="<inputTestLinuxUserName>"
$env:WINDOWSHOSTNAME="<inputTestWindowsHostName>"
$env:WINDOWSUSERNAME="<inputTestWindowsUserName>"
$env:PASSWORDSTRING="<inputTestHostPassword>"
```

Then open a self-hosted copy of PowerShell.

You should use `Invoke-Pester .\test\PSRP.Tests.ps1` to run the PSRP test on three different platforms to cover all the tests.