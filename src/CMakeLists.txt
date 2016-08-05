cmake_minimum_required(VERSION 2.8.4)
project(PSRP_PROVIDER)

# Export commands for auto-completion engines
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_compile_options(-Wall -Werror -fPIC)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
	add_definitions(-Dmacos)
	set(CMAKE_ICONV iconv)
endif ()

set(OMI ../omi/Unix)

find_package(Threads REQUIRED)
link_directories(${OMI}/output/lib)

add_library(psrpomiprov SHARED
	Shell.c
	Command.c
	module.c
	schema.c
	xpress.c
	BufferManipulation.c
        coreclrutil.cpp)
target_link_libraries(psrpomiprov
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
	crypto
	${CMAKE_ICONV})
target_include_directories(psrpomiprov PRIVATE ${OMI}/output/include ${OMI} ${OMI}/common)
target_compile_options(psrpomiprov PRIVATE -fvisibility=hidden -fno-strict-aliasing)

add_custom_command(TARGET psrpomiprov POST_BUILD
	COMMAND ${OMI}/output/bin/omireg $<TARGET_FILE:psrpomiprov> -n interop --hosting @requestor-shell@)
#	COMMAND ${OMI}/output/bin/omireg $<TARGET_FILE:psrpomiprov> -n interop)

add_custom_target(gen DEPENDS schema.mof
	COMMAND ${OMI}/output/bin/omigen -C ${OMI}/share/networkschema/CIM_Schema.mof schema.mof Shell Command)
