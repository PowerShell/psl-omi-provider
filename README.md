PowerShell Remoting Protocol [![Build Status](https://travis-ci.org/PowerShell/psl-omi-provider.svg?branch=master)](https://travis-ci.org/PowerShell/psl-omi-provider)
============================

PSRP communication is tunneled through the [Open Management
Infrastructure (OMI)][OMI] using this OMI provider.

[OMI]: https://github.com/Microsoft/omi

Two parts of PSRP are available - server and client.  The client PSRP part (libpsrpclient) is under development, and will be distributed together with PowerShell. The server PSRP package is available now and should be installed after installing [OMI][] as it is an OMI provider.

Get PSRP-Server
===============

You can download and install PSRP-Server from following links:

| Platform     | Releases           | Link                             |
|--------------|--------------------|----------------------------------|
| Linux        | Debian             | [psrp-1.4.1-28.universal.x64.deb] |
| Linux        | RPM                | [psrp-1.4.1-28.universal.x64.rpm] |

[psrp-1.4.1-28.universal.x64.deb]: https://github.com/PowerShell/psl-omi-provider/releases/download/v1.4.1-28/psrp-1.4.1-28.universal.x64.deb
[psrp-1.4.1-28.universal.x64.rpm]: https://github.com/PowerShell/psl-omi-provider/releases/download/v1.4.1-28/psrp-1.4.1-28.universal.x64.rpm

Alternatively, you can now also download from Microsoft Repo. Instructions
on setting this up can be found [here](https://technet.microsoft.com/en-us/windows-server-docs/compute/Linux-Package-Repository-for-Microsoft-Software).  Follow the instructions for your platform.  You can then use your platform's package tool to install OMI (i.e. "sudo apt-get install omi-psrp-server", or "sudo yum install omi-psrp-server").

Package Requirement
-------------------

Prior to installing PSRP, make sure that [OMI][] and [PowerShell][] are installed.

Development Environment
=======================

Toolchain Setup
---------------

PSRP has [OMI][OMI] as a submodule so install those dependencies first. Instructions are located in the [build-omi README.md][build-omi-readme].

PSRP requires the following additional dependent packages:

- On Ubuntu 14.04 and Ubuntu 16.04
```sh
sudo apt-get install cmake
```
- On CentOS 7.x
```sh
sudo yum install cmake
```
Also install [PowerShell][] from the latest release per their instructions.

[build-omi-readme]: https://github.com/Microsoft/Build-omi/blob/master/README.md#dependencies-to-build-a-native-package
[PowerShell]: https://github.com/PowerShell/PowerShell

Git Setup
---------

PSRP has a submodule, so clone recursively.

```sh
git clone --recursive https://github.com/PowerShell/psl-omi-provider.git
```

Building
--------

Run `make debug` or `make release` to build OMI and the provider.

In debug mode, this script first builds OMI in developer mode:

```sh
pushd omi/Unix
./configure --dev
make -j
popd
```

Then it builds and registers the provider:

```sh
pushd src
cmake -DCMAKE_BUILD_TYPE=Debug .
make -j
popd
```

In release mode, this script will build omi, the provider, and packages (for distribution).

Running
=======

Connecting from Windows to Linux
--------------------------------

Some initial setup on Windows is required. Open an administrative command
prompt and execute the following:

```cmd
winrm set winrm/config/Client @{TrustedHosts="*"}
```

> You can also set the `TrustedHosts` to include the target's IP address.

Then on Linux, launch `omiserver` (if you installed OMI from its installation package, omiserver
is probably already running):

```sh
./run.sh
```

Now in a PowerShell prompt on Windows (opened after setting the WinRM client
configurations):

```powershell
$cred = Get-Credential
$o = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
Enter-PSSession -ComputerName <IP address of Linux machine> -Credential $cred -Authentication basic -UseSSL -SessionOption $o
```

The IP address of the Linux machine can be obtained with:

```sh
ip -f inet addr show dev eth0
```

Connecting from Linux to Windows
--------------------------------

For Centos 7.3 and later, and Ubuntu 16.04 or later, Powershell supports Secure Protected Negotiated authentication (SPNEGO). 
This allows the use of NTLM based authentication and encryption of traffic over the http connection.  Use of SPNEGO authentication is
more secure than basic authentication on http, and less complex than https.  Support of NTLM authentication on MacOS is deprecated by apple. 
We are currently working on Kerberos protocol support which will allow SPNEGO or direct Kerberos authentication on MacOS as well as at least some
older Linux distributions.

In order to perform NTLM authentication there must be matching credentials on both ends of the transaction. The necessary setup of the ntlm credentials
for both server and client is described in the document [setup-ntlm-omi]( https://github.com/Microsoft/omi/blob/master/Unix/doc/setup-ntlm-omi.md). 

Note that the server side will need an additional registry setting to enable administrators, other than the built in administrator, to connect using NTLM. Refer to the LocalAccountTokenFilterPolicy registry setting under Negotiate Authentication in [Authentication for Remote Connections]( https://msdn.microsoft.com/en-us/library/aa384295(v=vs.85).aspx )

If you are not using SPNEGO authentication, or wish to use basic authentication on http, the WinRM server needs to be configured to allow unencrypted traffic
and accept basic authentication for inbound connections. *Note that this sends passwords over unencrypted http. We do not recommend it*.  If the http socket is enabled
and basic authentication is allowed, there is currently no way to prevent the use of basic authentication over http, which exposes passwords.  

To enable basic auth, on Windows in an administrative command prompt run:
```cmd
winrm set winrm/config/Service/Auth @{Basic="true"}
```
Basic authentication has acceptable security over https, but all communications using basic authentication over http are unencrypted. Connections using 
SPNEGO authentication are encrypted, and have acceptable security. 

To enable unencrypted communication over http on Windows in an administrative command prompt you must also run:
```cmd
winrm set winrm/config/Service @{AllowUnencrypted="true"}
```
Basic authentication with WinRM can only access local machine accounts so you will need to create a local account on your Windows machine that is part of the administrator group.a SPNEGO connections can use domain credentials. 


Building this repository generates two new binaries that need to be picked up instead of the ones included by PowerShell for Linux itself. 
Run PowerShell as:

```sh
export LD_LIBRARY_PATH=psl-omi-provider-path/src:psl-omi-provider-path/omi/Unix/output/lib:${LD_LIBRARY_PATH} && powershell
```

where psl-omi-provider-path is where you enlisted your psl-omi-provider code.

Now in PowerShell prompt on Linux you can connect to Windows using this command:

```powershell
$cred = Get-Credential
Enter-PSSession -ComputerName <IP address of windows machine> -Credential $cred -Authentication basic
```

