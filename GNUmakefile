# GNU makefile for building PSRP providers
#
# Two targets exist:
# 1. 'make debug' to build PSRP in debug mode,
# 2. 'make release' to build PSRP in release mode

all :
	@echo "Please issue 'make debug' or 'make release'"

debug :
	cd libmi/Unix; ./configure --dev; make -j
	cd src; cmake -DULINUX_SSL=0 -DCMAKE_BUILD_TYPE=Debug .; make -j; make reg


release-ulinux :
	cd libmi/Unix; ./configure --enable-system-build --disable-ssl-0.9.8; make -j
	cd src; cmake -DULINUX_SSL=1 .; make -j

release :
	cd libmi/Unix; ./configure --enable-microsoft; make -j
	cd src; cmake -DULINUX_SSL=0 .; make -j

clean :
	cd libmi/Unix; make clean; rm -rf output/ output_openssl_1.0.0/ output_openssl_1.1.0/ GNUmakefile
	cd src; make clean
	rm -rf target/
