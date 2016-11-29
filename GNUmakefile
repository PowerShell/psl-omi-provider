# GNU makefile for building PSRP providers
#
# Two targets exist:
# 1. 'make debug' to build PSRP in debug mode,
# 2. 'make release' to build PSRP in release mode

all :
	@echo "Please issue 'make debug' or 'make release'"

debug :
	cd omi/Unix; ./configure --dev; make -j
	cd src; cmake -DCMAKE_BUILD_TYPE=Debug .; make -j; make reg


release :
	cd omi/Unix; ./configure; make -j
	cd src; cmake .; make -j
	cd pal/build/; ./configure --enable-system-build
	cd installbuilder/; make
