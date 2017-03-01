cmake_minimum_required(VERSION 2.8.11)
project(PSRP)

# Export commands for auto-completion engines
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# OMI uses some GNU extensions that cannot be found in header files
# without defining this
add_definitions(-D_GNU_SOURCE)

# Dependent on the threading library. Nothing 
# equivalent for iconv unfortunately
find_package(Threads REQUIRED)

# Search OpenSSL
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
	find_package(openssl REQUIRED)
	message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
	message(STATUS "OpenSSL Libraries ${OPENSSL_LIBRARIES}")
	message(STATUS "OpenSSL Include dirs ${OPENSSL_INCLUDE_DIRS}")
elseif (ULINUX_SSL EQUAL 1)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(OPENSSL REQUIRED openssl)
	set(OPENSSL_LIBRARIES "-L/usr/local_ssl_1.0.0/lib64 -lssl -lcrypto" )
	set(OPENSSL_INCLUDES "/usr/local_ssl_1.0.0/inc" )
	message(STATUS "Using private openssl in ${OPENSSL_LIBRARIES}")
	message(STATUS "OpenSSL Libraries ${OPENSSL_LIBRARIES}")
	message(STATUS "OpenSSL Include dirs ${OPENSSL_INCLUDE_DIRS}")
else ()
	find_package(PkgConfig REQUIRED)
	pkg_search_module(OPENSSL REQUIRED openssl)
	message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
	message(STATUS "OpenSSL Libraries ${OPENSSL_LIBRARIES}")
	message(STATUS "OpenSSL Include dirs ${OPENSSL_INCLUDE_DIRS}")
endif ()

# Add the necessary complication options that OMI needs as we are pulling in a bunch of
# their libraries and headers
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC -fvisibility=hidden -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -fPIC -fvisibility=hidden -fno-strict-aliasing")

# OMI base directory
set(OMI ../omi/Unix)

# OMI build output is different when building with the ULINUX options
# OSX does not have ULINUX so override that
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(OMI_OUTPUT ${OMI}/output)
elseif (ULINUX_SSL EQUAL 1)
	set(OMI_OUTPUT ${OMI}/output_openssl_1.0.0)
else ()
	set(OMI_OUTPUT ${OMI}/output)
endif ()
message(STATUS "OMI directory is ${OMI}")
message(STATUS "OMI output directory is ${OMI_OUTPUT}")

# The search path for shared libraries
link_directories("${OMI_OUTPUT}/lib" "${OPENSSL_LIBRARIES}")

# Some global settings that differ depending on 
# platform being OSX or not
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# OMI requires 'macos' to be defined if we are building on 
	# this platform
	add_definitions(-Dmacos)

	# cannot just find iconv library on osx so 
	# set this up manually
	set(CMAKE_ICONV iconv)

	# couple of custom commands need the LD_LIBRARY_PATH
	# which is of course different on OSX
	set(OUR_LD_PATH export DYLD_LIBRARY_PATH)

	# set rpath to @rpath on mac
	set(CMAKE_MACOSX_RPATH 1)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

else ()
	# Normal LD_LIBRARY_PATH for linuxes for various 
	# custom commands.
	set(OUR_LD_PATH export LD_LIBRARY_PATH)

endif ()




# ##########################################
# 
# PSRP CLIENT specific configuration
#
# ##########################################

add_library(psrpclient SHARED
	Client.c
	xpress.c
	BufferManipulation.c
	schema.c
	Utilities.c
	)

# Dependent libraries are from OMI as well as threading
# and iconv.
target_link_libraries(psrpclient
	mi
	base
	pal
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_ICONV}
	)

# for non-osx platforms we need to set RPATH to $ORIGIN so it finds 
# the necessary libraries in the same directory as itself.
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set_property(TARGET psrpclient PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)
	set_property(TARGET psrpclient PROPERTY INSTALL_RPATH "$ORIGIN")
endif ()

# Standard OMI include path search
target_include_directories(psrpclient PRIVATE 
	${OMI_OUTPUT}/include 
	${OMI} 
	${OMI}/common
	${OPENSSL_INCLUDE_DIRS})

# This custom command uses a tool from OMI that tests all library functions 
# can be resolved. If it fails it means we are missing a dependent library
add_custom_command(TARGET psrpclient POST_BUILD
	COMMAND ${OUR_LD_PATH}=${OMI_OUTPUT}/lib && ${OMI_OUTPUT}/bin/chkshlib $<TARGET_FILE:psrpclient>)


# ##########################################
#
# PSRP PROVIDER specific configuration
#
# ##########################################

add_library(psrpomiprov SHARED
	Shell.c
	Command.c
	module.c
	schema.c
	xpress.c
	BufferManipulation.c
	coreclrutil.cpp
	Utilities.c
	)

target_link_libraries(psrpomiprov
	mi
	base
	pal
	${CMAKE_THREAD_LIBS_INIT}
	pam
	${OPENSSL_LIBRARIES}
	dl
	${CMAKE_ICONV})

# for non-osx platforms we need to set the RPATH to the omi lib path
# which is where it finds the location of omi and crypto libriaries
# it has a dependency on.
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set_property(TARGET psrpomiprov PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)
	set_property(TARGET psrpomiprov PROPERTY INSTALL_RPATH  "/opt/omi/lib")
endif ()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# OSX builds require include paths into OMI only
	target_include_directories(psrpomiprov PRIVATE
		${OMI_OUTPUT}/include 
		${OMI} 
		${OMI}/common
		${OPENSSL_INCLUDE_DIRS})
elseif (ULINUX_SSL EQUAL 1)
	# for ULINUX, we need to point include path at our private SSL install
	# as well as the OMI include directories
	target_include_directories(psrpomiprov PRIVATE
		/usr/local_ssl_1.0.0/include 
		${OMI_OUTPUT}/include
		${OMI} 
		${OMI}/common
		${OPENSSL_INCLUDE_DIRS})
else ()
	# Regular builds require include paths into OMI only
	target_include_directories(psrpomiprov PRIVATE
		${OMI_OUTPUT}/include 
		${OMI} 
		${OMI}/common
		${OPENSSL_INCLUDE_DIRS})
endif ()

# This custom command uses a tool from OMI that tests all library functions 
# can be resolved. If it fails it means we are missing a dependent library
add_custom_command(TARGET psrpomiprov POST_BUILD
	COMMAND  ${OUR_LD_PATH}=${OMI_OUTPUT}/lib && ${OMI_OUTPUT}/bin/chkshlib $<TARGET_FILE:psrpomiprov>)



# ##########################################
#
# Register the PSRP provider with OMI. Note this is a special shell provider
# which is different from most other providers out there.
#
# ##########################################

add_custom_target(reg
	COMMAND  ${OUR_LD_PATH}=${OMI_OUTPUT}/lib && ${OMI_OUTPUT}/bin/omireg $<TARGET_FILE:psrpomiprov> -n interop --hosting @requestor-shell@)


# ##############################################
#
# Generates template files from an updated .mof
#
# ##############################################

add_custom_target(gen DEPENDS schema.mof
	COMMAND  ${OUR_LD_PATH}=${OMI_OUTPUT}/lib && ${OMI_OUTPUT}/bin/omigen -C ${OMI}/share/networkschema/CIM_Schema.mof schema.mof Shell Command)

