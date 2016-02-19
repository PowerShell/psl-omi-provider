cmake_minimum_required(VERSION 2.8.4)
project(PSRP_PROVIDER)

set(CMAKE_BUILD_TYPE Debug)

# Export commands for auto-completion engines
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Werror -fPIC)

set(OMI ../omi/Unix)

find_package(Threads REQUIRED)
find_package(Boost 1.54.0 REQUIRED COMPONENTS filesystem system)
link_directories(${OMI}/output/lib)

add_library(pshost SHARED coreclrutil.cpp)
target_link_libraries(pshost PRIVATE dl ${Boost_LIBRARIES})
target_include_directories(pshost PRIVATE ${Boost_INCLUDE_DIR})
target_compile_options(pshost PRIVATE -std=c++11)

add_library(psrpomiprov SHARED
	Shell.c
	Command.c
	module.c
	schema.c
	xpress.c
	BufferManipulation.c)
target_link_libraries(psrpomiprov pshost
	miapi
	omi_error
	wsman
	xmlserializer
	protocol
	sock
	provmgr
	wql
	base
	pal
	${CMAKE_THREAD_LIBS_INIT}
	dl
	pam
	ssl
	crypto)
target_include_directories(psrpomiprov PRIVATE ${OMI}/output/include ${OMI} ${OMI}/common)
target_compile_options(psrpomiprov PRIVATE -fvisibility=hidden -fno-strict-aliasing)

add_custom_command(TARGET psrpomiprov POST_BUILD
	COMMAND ${OMI}/output/bin/omireg $<TARGET_FILE:psrpomiprov> -n interop --hosting @requestor-shell@)
#	COMMAND ${OMI}/output/bin/omireg $<TARGET_FILE:psrpomiprov> -n interop)

add_custom_target(gen DEPENDS schema.mof
	COMMAND ${OMI}/output/bin/omigen -C ${OMI}/share/networkschema/CIM_Schema.mof schema.mof Shell Command)
