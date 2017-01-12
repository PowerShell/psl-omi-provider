#! /bin/sh

# buildNuget.sh
#
# Build nuget packages for OMI (libmi)
#

# Abort on error

set -e

# Define necessary constants

build_location=/mnt/ostcdata/OSTCData/Builds/psrp/develop

subdir_linux=Linux_ULINUX_1.0_x64_64_Release
subdir_osx=Darwin_10.11_x64_64_Release

# We should be built from omi/Unix directory, which means that
# the "installbuilder" directory should live here.

if [ ! -d installbuilder ]; then
    echo "$0: Must be run from psrp repository root directory" 1>& 2
    exit 1
fi

# We don't have any version information for PSRP, so use Jenkinds build number

if [ -z "$PARENT_BUILD_NUMBER" ]; then
    echo "$0: Jenkins PARENT_BUILD_NUMBER variable is not defined; run via Jenkins" 1>& 2
fi

# Verify that we can access the build shares to get the packages we need

if [ ! -d ${build_location}/${PARENT_BUILD_NUMBER} ]; then
    echo "$0: Unable to locate build share at ${build_location}" 1>& 2
    exit 1
fi

build_linux=${build_location}/${PARENT_BUILD_NUMBER}/${subdir_linux}
build_osx=${build_location}/${PARENT_BUILD_NUMBER}/${subdir_osx}

if [ ! -d ${build_linux} ]; then
    echo "$0: Unable to locate OMI Linux build at: ${build_linux}"
    exit 1
fi

if [ ! -d ${build_osx} ]; then
    echo "$0: Unable to locate OMI OS/X build at: ${build_osx}"
    exit 1
fi

#
# Write out the spec file
#

rm -rf installbuilder/nuget
mkdir -p installbuilder/nuget
cd installbuilder/nuget

echo "Writing psrp.nuspec file ..."

cat > psrp.nuspec <<EOF
<?xml version="1.0" encoding="utf-8"?>
<package xmlns="http://schemas.microsoft.com/packaging/2011/10/nuspec.xsd">
  <metadata>
    <id>psrp</id>
    <version>1.0.0-alpha${PARENT_BUILD_NUMBER}</version>
    <authors>Microsoft</authors>
    <owners>Microsoft</owners>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>PowerShell WS-Man PSRP Client for Linux and OSX</description>
    <copyright>Copyright 2017 Microsoft</copyright>
    <dependencies>
      <group targetFramework=".NETStandard1.6">
        <dependency id="libmi" version="1.0.0-*" />
      </group>
    </dependencies>
  </metadata>
</package>
EOF

#
# Build directory structure and populate it
#
# Note, up above, that we are now in the installbuilder/nuget directory
#

mkdir -p runtimes/linux-x64/native runtimes/osx/native 

# Copy the appropriate files in place

cp ${build_linux}/libpsrpclient.so runtimes/linux-x64/native
cp ${build_osx}/libpsrpclient.dylib runtimes/osx/native

# Finally, invoke dotnet to build the nuget package

dotnet nuget pack psrp.nuspec

# All done

exit 0
