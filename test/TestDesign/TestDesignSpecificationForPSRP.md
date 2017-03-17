# PSRP Test Design Specification 

## Content

* [  Test Environment](#1)
	* [ Windows Test Environment](#1.1)
	* [ Linux Test Environment](#1.2)
	* [ Mac Test Environment](#1.3)
* [ Test Scope](#2)
	* [Test Target](#2.1)
	* [Test Protocols](#2.2)
	* [Restrictions](#2.3)
	* [ Dependencies](#2.4)
* [Test Suite Design](#3)
	* [Test Matrix](#3.1)
		* [ Basic authentication Mode](#3.1.1)
		* [ Negotiate authentication Mode](#3.1.2)
	* [Test Cases](#3.2)
		* [ Basic authentication Test](#3.2.1)
		* [ Negotiate authentication Test](#3.2.2)

## Test Environment

### Windows Test Environment

Current Supported Windows Operating Systems

| **Operating System**   | **Virtual Machines**            |
|------------------------|---------------------------------|
| Windows Server 2012 R2 | PSRP-W12R2-NO                   |
| Windows Server 2016    | PSRP-W2016-NO                   |


### Linux Test Environment

Current Supported Linux Operating Systems

| **Operating System**   | **Virtual Machines**            |
|------------------------|---------------------------------|
| Cent7X64               | psl-cent7x64-NO                 |
| Ubuntu16X64            | psl-ub16x64-NO                  |
| Ubuntu14X64            | psl-ub14x64-NO                  |


### Mac Test Environment

Current Supported Mac Operating Systems

| **Operating System**   | **Virtual Machines**            |
|------------------------|---------------------------------|
| Mac 1011               | psl-mac1011-NO                 |


## Test Scope
### Test Target
This test suite will test the server remote access of Windows and Linux, and test suite acts as client role (Which can be Windows, Linux and Mac platforms).

### Test Protocols
This test suite includes Basic Authentication and Negotiate Authentication between client and server to remote destination machine.
1.	Basic authentication
2.	Negotiate authentication

### Restrictions
1.  Mac machines are not supported in Negotiate Mode, They can only act as a client in Basic Mode.
2.  Ubuntu14X64 machines are not supported in Negotiate Mode, They can only act in Basic mode as both the client and server.

### Dependencies
PowerShell and OMI
Before we install the PSRP for test, we have to install related PowerShell and OMI providers first. 
		
## Test Suite Design

### Test Matrix

We logon the client and connect to the server remotely.

| **Client**   | **Server**            |
|------------------------|---------------------------------|
| Linux               | Linux                 |
| Linux               | Windows                 |
| Windows               | Linux                 |
| Mac               | Linux                 |
| Mac               | Windows                 |

If we divide the client and server into different platforms according to the test environments we mentioned above the matrix will be like below:
#### Basic authentication Mode

|**NO**| **Client**   | **Server**            |
|---|----------|---------------------------|
|1|Cent7X64 | Cent7X64|
|2|Cent7X64 | Ub16X64|
|3|Cent7X64 | Ub14X64|
|4|Ub16X64 | Ub16X64|
|5|Ub16X64 | Ub14X64|
|6|Ub16X64 | Cent7X64|
|7|Ub14X64 | Ub14X64|
|8|Ub14X64 | Cent7X64|
|9|Ub14X64 | Ub16X64|
|10|Cent7X64 | WindowsServer2012R2|
|11|Cent7X64| WindowsServer2016|
|12|Ub16X64 |WindowsServer2012R2|
|13|Ub16X64 | WindowsServer2016|
|14|Ub14X64 | WindowsServer2012R2|
|15|Ub14X64 |WindowsServer2016|
|16|WindowsServer2012R2 | Cent7X64|
|17|WindowsServer2012R2 | Ub16X64|
|18|WindowsServer2012R2| Ub14X64|
|19|WindowsServer2016 | Cent7X64|
|20|WindowsServer2016 | Ub16X64|
|21|WindowsServer2016 | Ub14X64|
|22|Mac | Cent7X64|
|23|Mac | Ub16X64|
|24|Mac | Ub14X64|
|25|Mac | WindowsServer2012R2|
|26|Mac | WindowsServer2016|

#### Negotiate authentication Mode

|**NO**| **Client**   | **Server**           |
|-----|--------------|---------------------|
|1|Cent7X64 | Cent7X64|
|2|Cent7X64 | Ub16X64|
|3|Ub16X64 | Ub16X64|
|4|Ub16X64 | Cent7X64|
|5|Cent7X64 | WindowsServer2012R2|
|6|Cent7X64 | WindowsServer2016|
|7|Ub16X64 | WindowsServer2012R2|
|8|Ub16X64| WindowsServer2016|
|9|WindowsServer2012R2 | Cent7X64|
|10|WindowsServer2012R2 | Ub16X64|
|11|WindowsServer2016 | Cent7X64|
|12|WindowsServer2016 | Ub16X64|

### Test Cases
 
**You can refer to the implemented test cases in github: [PSRP Test Cases](https://github.com/PowerShell/psl-omi-provider/blob/master/test/PSRP.Tests.ps1)**
	 
#### Basic Authentication Test
|**ID**| **Case Title** | **Pester Test Title** |
|-----|--------------|---------------------|
1 |[BasicModeToLinux] Powershell remoting over https with valid credentials should work |PowerShell from Windows/Linux/MacOS to Linux with basic authentication over https should work|
2 |[BasicModeToLinux] WSMan remoting over https with winrm and valid credentials should work |WSMan from Windows to Linux with basic authentication with winrm over https should work|
3 |[BasicModeToLinux] WSMan remoting over https with omicli and valid credentials should work |WSMan from Linux/MacOS to Linux with basic authentication with omicli over https should work|
4 |[BasicModeToLinux] Powershell remoting from Windows over https with bad username should throw exception |PowerShell from Windows to Linux with basic authentication with bad username over https should throw exception|
5 |[BasicModeToLinux] Powershell remoting from Linux over https with bad password should throw exception |PowerShell from Linux/MacOS to Linux with basic authentication with bad password over https should throw exception|
6 |[BasicModeToLinux] WSMan remoting over https with winrm bad user should throw exception  |WSMan from Windows to Linux with basic authentication with winrm with bad username over https should throw exception|
7 |[BasicModeToLinux] WSMan remoting over https with omicli and bad user should throw exception |WSMan from Linux/MacOS to Linux with basic authentication with omicli with bad username over https should throw exception|
8 |[BasicModeToLinux] WSMan remoting over https with winrm  with bad password should throw exception |WSMan from Windows to Linux with basic authentication with winrm with bad password over https should throw exception|
9 |[BasicModeToLinux] WSMan remoting over https with omicli  with bad password should throw exception |WSMan from Linux/MacOS to Linux with basic authentication with omicli with bad password over https should throw exception|
10 |[BasicModeToWindows] Powershell remoting over https with valid credentials should work |PowerShell from Linux/MacOS to Windows with basic authentication over http should work|
11 |[BasicModeToWindows] WSMan remoting over https with omicli with valid credentials should work |WSMan from Linux/MacOS to Windows with basic authentication by omicli over http should work|
12 |[BasicModeToWindows] Powershell remoting over http with bad user throw exception |  Need implement|
13 |[BasicModeToWindows] Powershell remoting over http with bad password  throw exception |  Need implement|
14 |[BasicModeToWindows] WSMan remoting over http with omicli  with bad user should throw exception |  Need implement|
15 |[BasicModeToWindows] WSMan remoting over http with omicli  with bad password  should throw exception |  Need implement|

#### Negotiate Authentication Test
|**ID**| **Case Title** | **Pester Test Title** |
|-----|--------------|---------------------|
16 |[NegotiateModeToLinux]  Powershell remoting over https with valid credentials should work |PowerShell from Windows/Linux/MacOS to Linux with negotiate authentication over https should work|
17 |[NegotiateModeToLinux]  WSMan remoting over https with winrm  with valid credentials should work |WSMan from Windows to Linux with negotiate authentication with winrm over https should work|
18 |[NegotiateModeToLinux]  WSMan remoting over https with omicli  with valid credentials should work |WSMan from Linux/MacOS to Linux with negotiate authentication with omicli over https should work|
19 |[NegotiateModeToLinux]  Powershell remoting over https with bad user throw exception |  Need implement|
20 |[NegotiateModeToLinux]  Powershell remoting over https with bad password  throw exception |  Need implement|
21 |[NegotiateModeToLinux]  WSMan remoting over https with winrm  with bad user should throw exception  |  Need implement|
22 |[NegotiateModeToLinux]  WSMan remoting over https with omicli  with bad user should throw exception |  Need implement|
23 |[NegotiateModeToLinux]  WSMan remoting over https with winrm  with bad password  should throw exception |  Need implement|
24 |[NegotiateModeToLinux]  WSMan remoting over https with omicli  with bad password  should throw exception |  Need implement|
25 |[NegotiateModeToWindows] Powershell remoting over http with valid credentials should work |PowerShell from Linux/MacOS to Windows with negotiate authentication over http should work
26 |[NegotiateModeToWindows] WSMan remoting over http with omicli  with valid credentials should work |  Need implement|
27 |[NegotiateModeToWindows] Powershell remoting over http with bad user throw exception |  Need implement|
28 |[NegotiateModeToWindows] Powershell remoting over http with bad password  throw exception |  Need implement|
29 |[NegotiateModeToWindows] WSMan remoting over http with omicli with bad user should throw exception |  Need implement|
30 |[NegotiateModeToWindows] WSMan remoting over http with omicli with bad password  should throw exception | Need implement|
