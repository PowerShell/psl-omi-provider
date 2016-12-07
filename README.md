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
| Linux        | Debian             | [psrp-1.0.0-0.universal.x64.deb] |
| Linux        | RPM                | [psrp-1.0.0-0.universal.x64.rpm] |

[psrp-1.0.0-0.universal.x64.deb]: https://github.com/PowerShell/psl-omi-provider/releases/download/v.1.0/psrp-1.0.0-0.universal.x64.deb
[psrp-1.0.0-0.universal.x64.rpm]: https://github.com/PowerShell/psl-omi-provider/releases/download/v.1.0/psrp-1.0.0-0.universal.x64.rpm

Package Requirement
-------------------

Prior to installing PSRP, make sure that [OMI][] and [PowerShell][] are installed.

Development Environment
=======================

Toolchain Setup
---------------

PSRP requires the following packages:

```sh
sudo apt-get install libpam0g-dev libssl-dev
```

Also install [PowerShell][] from the latest release per their instructions.

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

*NOTE: This is pre-release and does have plenty of known problems. We are pushing it to master so you can try it out.*
*Please don't file issues on crashes or hangs yet as it is probably already known about.*

WinRM server needs to be configured to allow unencrypted traffic and accept basic authentication for inbound connections.
*NOTE: This sends passwords over unencrypted http.*

We are working on SPNEGO authentication with encryption over HTTP and this will hopefully be in place soon.

On Windows in an administrative command prompt run:
```cmd
winrm set winrm/config/Service @{AllowUnencrypted="true"}
winrm set winrm/config/Service/Auth @{Basic="true"}
```

Basic authentication with WinRM can only access local machine accounts so you will need to create a local account on your Windows machine that is part of the administrator group.


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

