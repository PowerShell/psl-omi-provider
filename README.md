PowerShell Remoting Protocol [![Build Status](https://travis-ci.com/PowerShell/psl-omi-provider.svg?token=31YifM4jfyVpBmEGitCm&branch=master)](https://travis-ci.com/PowerShell/psl-omi-provider)
============================

PSRP communication is tunneled through the [Open Management
Infrastructure (OMI)][omi] using this OMI provider.

[omi]: https://github.com/PowerShell/omi

Environment
===========

Toolchain Setup
---------------

PSRP requires the following packages:

```sh
sudo apt-get install libpam0g-dev libssl-dev libcurl4-openssl-dev
```

Also install [PowerShell][] from the latest release per their instructions.

[powershell]: https://github.com/PowerShell/PowerShell

Git Setup
---------

PSRP has a submodule, so clone recursively.

```sh
git clone --recursive https://github.com/PowerShell/psl-omi-provider.git
```

Building
========

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

The provider maintains its own native host library to initialize the
CLR, but there are plans to refactor .NET's packaged host as a shared
library.

Running
-------

Some initial setup on Windows is required. Open an administrative command
prompt and execute the following:

```cmd
winrm set winrm/config/Client @{AllowUnencrypted="true"}
winrm set winrm/config/Client @{TrustedHosts="*"}
```

> You can also set the `TrustedHosts` to include the target's IP address.

Then on Linux, launch `omiserver` (after building with the
instructions above):

```sh
./run.sh
```

Now in a PowerShell prompt on Windows (opened after setting the WinRM client
configurations):

```powershell
Enter-PSSession -ComputerName <IP address of Linux machine> -Credential $cred -Authentication basic
```

The IP address of the Linux machine can be obtained with:

```sh
ip -f inet addr show dev eth0
```
