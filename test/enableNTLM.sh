#!/usr/bin/env bash

# Let's quit on interrupt of subcommands
trap '
  trap - INT # restore default INT handler
  echo "Interrupted"
  kill -s INT "$$"
' INT

if [ $# -ne 1 ]; then 
    echo -e "Need rootpassword\nUsage:enableNTLM.sh rootpassword"
    exit 2
fi

vmname=`hostname`
rootpassword=$1
echo $vmname":root:"$rootpassword>$HOME/ccache

# Get OS specific asset ID and package name
case "$OSTYPE" in
    linux*)
        source /etc/os-release
        case "$ID" in
            centos*)
                sudo yum update -y
                sudo yum install epel-release -y
                sudo yum install gssntlmssp.x86_64 -y
                sudo yum install krb5-libs -y
				sudo yum install krb5-devel -y
                [ ! -f "/lib64/libcrypto.so.1.0.0" ] && sudo ln -s /lib64/libcrypto.so.10 /lib64/libcrypto.so.1.0.0
                [ ! -f "/lib64/libcrypto.so.1.0.0" ] && sudo ln -s /lib64/libssl.so.10 /lib64/libssl.so.1.0.0
                [[ $(sudo firewall-cmd --list-all) =~ "5986" ]] || sudo firewall-cmd --zone=public --add-port=5986/tcp --permanent
                [[ $(sudo firewall-cmd --list-all) =~ "5985" ]] || sudo firewall-cmd --zone=public --add-port=5985/tcp --permanent
                sudo firewall-cmd --reload
				# if not contains active, we need to add
                [[ $(sudo firewall-cmd --list-all) =~ "active" ]] || sudo firewall-cmd --zone=public --add-interface=eth0
                sudo systemctl restart firewalld
                ;;
            ubuntu)
                sudo apt-get update
                sudo apt-get install cifs-utils -y
                sudo apt-get install gss-ntlmssp -y
                sudo apt-get install libkrb5-dev -y
                sudo apt-get install libnss-winbind -y
                sudo apt-get install libpam-winbind -y
                sudo cp -u /etc/gss/mech.d/mech.ntlmssp /etc/gss/mech.d/mech.ntlmssp.conf
                sudo sed -i 's/${prefix}/\/usr/g' /etc/gss/mech.d/mech.ntlmssp.conf
                ;;
            *)
                echo "$NAME is not supported!" >&2
                exit 2
        esac
        ;;
    darwin*)
        echo "Runtime is darwin, we don't support Mac OS as server now!"
        exit 2
        ;;
    *)
        echo "$OSTYPE is not supported!" >&2
        exit 2
        ;;
esac

success=$?

if [[ "$success" != 0 ]]; then
    echo "Enable NTLM failed." >&2
    exit "$success"
else
    echo "Congratulations! NTLM are enabled on server. You can run \"sudo systemctl stop omid\" to stop omiserver and start NTLM authentication with \"sudo NTLM_USER_FILE=$HOME/ccache /opt/omi/bin/omiserver --loglevel 4 --httptrace -l\""
	echo "How to test NTLM works fine on localhost:"
	echo "\"sudo /opt/omi/bin/omicli id -u \"$vmname\root\" -p $rootpassword --auth NegoWithCreds --hostname $vmname --port 5986 --encryption https\""
	exit "$success"
fi
