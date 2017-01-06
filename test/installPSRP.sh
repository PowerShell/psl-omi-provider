#!/usr/bin/env bash

# Let's quit on interrupt of subcommands
trap '
  trap - INT # restore default INT handler
  echo "Interrupted"
  kill -s INT "$$"
' INT

if [ $# -ne 1 ]; then 
    echo -e "Need redmondpassword\nUsage:installPSRP.sh redmondpassword"
    exit 2
fi

chmod +x ./installpowershell.sh
./installpowershell.sh

redmondpassword=$1
isMacOS=false
omiversion="1.1.0-52"
psrpversion="3"
powershellDir="/opt/microsoft/powershell/6.0.0-alpha.14"
powershellDirForMac="/usr/local/microsoft/powershell/6.0.0-alpha.14"
realdataDir="//wsscnafiler43/ostcdata$"
get_omifolder() {
    echo "/download/OSTCData/Builds/omi/develop/$1/$2/$3"
}

get_psrpfolder() {
    echo "/download/OSTCData/Builds/psrp/develop/$1/$2"
}

# Get OS specific asset ID and package name
case "$OSTYPE" in
    linux*)
        source /etc/os-release
        case "$ID" in
            centos*)
                if ! hash openssl 2>/dev/null; then
                    echo "openssl not found, installing..."
                    sudo yum install -y openssl
                fi
                
                if ! hash mount.cifs 2>/dev/null; then
                    echo "mount.cifs not found, installing..."
                    sudo yum install -y cifs-utils
                fi

                platfrom=Linux_ULINUX_1.0_x64_64_Release
                ov=`openssl version|cut -d' ' -f2|cut -c1`
                case "$ov" in
                 1) opensslversion="openssl_1.0.0" ;;
                 *) opensslversion="openssl_0.9.8" ;;
                esac
                ;;
            ubuntu)
                if ! hash openssl 2>/dev/null; then
                    echo "openssl not found, installing..."
                    sudo apt-get install -y openssl
                fi
                
                if ! hash mount.cifs 2>/dev/null; then
                    echo "mount.cifs not found, installing..."
                    sudo apt-get install -y cifs-utils
                fi

                case "$VERSION_ID" in
                    14.04)
                        platfrom=Linux_ULINUX_1.0_x64_64_Release
                        ;;
                    16.04)
                        platfrom=Linux_ULINUX_1.0_x64_64_Release
                        ;;
                    *)
                        echo "Ubuntu $VERSION_ID is not supported!" >&2
                        exit 2
                esac
                
                ov=`openssl version|cut -d' ' -f2|cut -c1`
                case "$ov" in
                 1) opensslversion="openssl_1.0.0" ;;
                 *) opensslversion="openssl_0.9.8" ;;
                esac
                ;;
            *)
                echo "$NAME is not supported!" >&2
                exit 2
        esac
        ;;
    darwin*)
        echo "Runtime is darwin!"
        platfrom=Darwin_10.11_x64_64_Release
        opensslversion=""
        isMacOS=true
        powershellDir=$powershellDirForMac
        ;;
    *)
        echo "$OSTYPE is not supported!" >&2
        exit 2
        ;;
esac

if sudo bash -c '[ ! -d "/download" ]' ; then
    sudo mkdir /download
fi

if [ "$isMacOS" = "true" ]; then
    if sudo bash -c '[ ! -d "/opt/omi" ]' ; then
    sudo mkdir /opt/omi
    fi
    if sudo bash -c '[ ! -d "/opt/omi/bin" ]' ; then
    sudo mkdir /opt/omi/bin
    fi
    if sudo bash -c '[ ! -d "/opt/omi/lib" ]' ; then
    sudo mkdir /opt/omi/lib
    fi
    # Mac OS don't have mount_cifs, so use mount_smbfs
    omifolder=$(get_omifolder "$omiversion" "$platfrom" "$opensslversion")
    echo "mounting from $realdataDir folder to omi folder: $omifolder"
    sudo mount -t smbfs '//redmond.corp.microsoft.com;scxsvc:'"$redmondpassword"'@wsscnafiler43/ostcdata$' /download
    sudo cp -f $omifolder"omicli" /opt/omi/bin
    sudo cp -f $omifolder"libmi.dylib" /opt/omi/lib
    sudo cp -f $omifolder"libmi.dylib" $powershellDir
    sudo umount /download

    psrpfolder=$(get_psrpfolder "$psrpversion" "$platfrom")
    echo "mounting from $realdataDir folder to psrp folder: $psrpfolder"
    sudo mount -t smbfs '//redmond.corp.microsoft.com;scxsvc:'"$redmondpassword"'@wsscnafiler43/ostcdata$' /download
    echo "Copying psrpclient ..."
    sudo cp -f $psrpfolder/libpsrpclient.dylib $powershellDir
    sudo umount /download
else
    omifolder=$(get_omifolder "$omiversion" "$platfrom" "$opensslversion")
    echo "mounting from $realdataDir folder to omi folder: $omifolder"
    sudo mount -t cifs $realdataDir /download -o username=scxsvc,password="$redmondpassword",domain=redmond.corp.microsoft.com
    sudo cp -u $omifolder/* $(pwd)
    sudo umount /download

    psrpfolder=$(get_psrpfolder "$psrpversion" "$platfrom")
    echo "mounting from $realdataDir folder to psrp folder: $psrpfolder"
    sudo mount -t cifs $realdataDir /download -o username=scxsvc,password="$redmondpassword",domain=redmond.corp.microsoft.com
    sudo cp -u $psrpfolder/* $(pwd)
    sudo umount /download
fi

# Installs OMI and PSRP package
case "$OSTYPE" in
    linux*)
        source /etc/os-release
        # Install dependencies
        echo "Installing OMI and PSRP with sudo..."
        case "$ID" in
            centos)
                # yum automatically resolves dependencies for local packages
                omipackage=omi-$omiversion.ulinux.x64.rpm
                if [[ ! -r "$omipackage" ]]; then
                    echo "ERROR: $omipackage failed to download! Aborting..." >&2
                    exit 1
                fi
                sudo rpm -i "./$omipackage"
                
                psrppackage=psrp-1.0.0-0.universal.x64.rpm
                if [[ ! -r "$psrppackage" ]]; then
                    echo "ERROR: $psrppackage failed to download! Aborting..." >&2
                    exit 1
                fi
                sudo rpm -i "./$psrppackage"
                
                echo "Copying omicli and psrpclient ..."
                #sudo cp -u libmi.so $powershellDir
                sudo cp -u libpsrpclient.so $powershellDir
                ;;
            ubuntu)
                # dpkg does not automatically resolve dependencies, but spouts ugly errors
                omipackage=omi-$omiversion.ulinux.x64.deb
                if [[ ! -r "$omipackage" ]]; then
                    echo "ERROR: $omipackage failed to download! Aborting..." >&2
                    exit 1
                fi
                sudo dpkg -i "./$omipackage"
                
                psrppackage=psrp-1.0.0-0.universal.x64.deb
                if [[ ! -r "$psrppackage" ]]; then
                    echo "ERROR: $psrppackage failed to download! Aborting..." >&2
                    exit 1
                fi
                sudo dpkg -i "./$psrppackage" &> /dev/null
                
                echo "Copying omicli and psrpclient ..."
                #sudo cp -u libmi.so $powershellDir
                sudo cp -u libpsrpclient.so $powershellDir
                # Resolve dependencies
                sudo apt-get install -f
                ;;
            *)
        esac
        ;;
    darwin*)
        echo "Done Copying omicli and psrpclient ..."
        ;;
esac
success=$?

if [[ "$success" != 0 ]]; then
    echo "Congratulations! OMI and PSRP are installed." >&2
    exit "$success"
fi
