#!/usr/bin/env bash

# Let's quit on interrupt of subcommands
trap '
  trap - INT # restore default INT handler
  echo "Interrupted"
  kill -s INT "$$"
' INT


# Get OS specific asset ID and package name
case "$OSTYPE" in
    linux*)
        source /etc/os-release
        case "$ID" in
            centos*)
                psrpexist=`rpm -qa|grep "omi-psrp-server"|wc -c`
                if  [ $psrpexist -ne 0 ]; then
                    echo "omi-psrp-server found, uninstalling psrp..."
                    sudo rpm -e `rpm -qa|grep omi-psrp-server`
                fi
				
				omiexist=`rpm -qa|grep "omi"|wc -c`
                if  [ $omiexist -ne 0 ]; then
                    echo "omiserver found, uninstalling omi..."
                    sudo rpm -e `rpm -qa|grep omi`
                fi
				
				echo "Removing omi folders..."
				rm -rf "/var/opt/omi"
				rm -rf "/etc/opt/omi"
				
                if  hash powershell 2>/dev/null; then
                    echo "powershell found, uninstalling..."
                    sudo rpm -e powershell
                fi
				
				echo "Removing powershell folders..."
				rm -rf "/opt/microsoft/powershell"
                echo "Done remove powershell,omi and psrp"
                ;;
            ubuntu)
				psrpexist=`dpkg -l|grep "PowerShell Remoting Protocol"|wc -c`
                if  [ $psrpexist -ne 0 ]; then
                    echo "omi-psrp-server found, uninstalling omi-psrp-server..."
                    sudo dpkg --purge omi-psrp-server
                fi
				
                omiexist=`dpkg -l|grep "Open Management Infrastructure"|wc -c`
                if  [ $omiexist -ne 0 ]; then
                    echo "omiserver found, uninstalling omi..."
                    sudo dpkg --purge omi
                fi
				
				echo "Removing omi folders..."
				rm -rf "/var/opt/omi"
				rm -rf "/etc/opt/omi"
				
                if  hash powershell 2>/dev/null; then
                    echo "powershell found, uninstalling..."
                    sudo dpkg --purge powershell
                fi
				
				echo "Removing powershell folders..."
				rm -rf "/opt/microsoft/powershell"
                echo "Done remove powershell,omi and psrp"
                ;;
            *)
                echo "$NAME is not supported!" >&2
                exit 2
        esac
        ;;
    darwin*)
        echo "Removing powershell..."
		sudo rm -rf "/usr/local/bin/powershell"
		sudo rm -rf "/usr/local/microsoft/powershell"
		sudo rm -rf "/usr/local/share/man/man1/powershell.1.gz"
		sudo pkgutil --forget powershell
        ;;
    *)
        echo "$OSTYPE is not supported!" >&2
        exit 2
        ;;
esac
success=$?

if [[ "$success" != 0 ]]; then
    echo "PowerShell, OMI and PSRP uninstall failed." >&2
    exit "$success"
else
    echo "Congratulations! PowerShell, OMI and PSRP are uninstalled."
    exit "$success"
fi
