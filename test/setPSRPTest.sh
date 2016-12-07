#!/bin/bash

while [ $# -gt 0 ]; do
  case "$1" in
    --linux=*)
      linux="${1#*=}"
	  linux=${linux////:}
	  IFS=':' read -r -a linuxarr <<< "$linux"
	  echo "Setting LINUXHOSTNAME=${linuxarr[0]}"
	  export LINUXHOSTNAME="${linuxarr[0]}"
	  echo "Setting LINUXUSERNAME=${linuxarr[1]}"
	  export LINUXUSERNAME="${linuxarr[1]}"
	  echo "Setting LINUXPASSWORDSTRING=${linuxarr[2]}"
	  export LINUXPASSWORDSTRING="${linuxarr[2]}"
      ;;
    --windows=*)
      windows="${1#*=}"
	  windows=${windows////:}
	  IFS=':' read -r -a windowsarr <<< "$windows"
	  echo "Setting WINDOWSHOSTNAME:${windowsarr[0]}"
	  export WINDOWSHOSTNAME="${windowsarr[0]}"
	  echo "Setting WINDOWSUSERNAME:${windowsarr[1]}"
	  export WINDOWSUSERNAME="${windowsarr[1]}"
	  echo "Setting WINDOWSPASSWORDSTRING:${windowsarr[2]}"
	  export WINDOWSPASSWORDSTRING="${windowsarr[2]}"
      ;;
    *)
      printf "***************************\n"
      printf "* Error: Invalid argument.*\n"
      printf "***************************\n"
      exit 1
  esac
  shift
done

printf "Done setting for linux is %s\n" "$linux"
printf "Done setting for windows is %s\n" "$windows"