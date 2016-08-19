PowerShell Remoting Protocol [![Build Status](https://travis-ci.org/PowerShell/psl-omi-provider.svg?branch=master)](https://travis-ci.org/PowerShell/psl-omi-provider)
============================

PSRP communication is tunneled through the [Open Management
Infrastructure (OMI)][OMI] using this OMI provider.

[OMI]: https://github.com/Microsoft/omi

Two versions of PSRP are available - server and client.  The client PSRP package is under development, and will be distributed together with OMI. The server PSRP package is available now.

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

Run `./build.sh` to build OMI and the provider.

This script first builds OMI in developer mode:

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

Running
=======

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
$o = New-PSSessionOption -SkipCACheck -SkipRevocationCheck -SkipCNCheck
Enter-PSSession -ComputerName <IP address of Linux machine> -Credential $cred -Authentication basic -UseSSL -SessionOption $o
```

The IP address of the Linux machine can be obtained with:

```sh
ip -f inet addr show dev eth0
```
