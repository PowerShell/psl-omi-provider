TOP = $(shell cd .. ; pwd)
include $(TOP)/pal/build/config.mak

DISTRO_TYPE = $(PF)
ifeq ($(PF),Linux)
DISTRO_TYPE = $(PF)_$(PF_DISTRO)
endif

PSRP_VERSION = 1.4.5
PSRP_PATCH_LEVEL = $(shell if [ -z "$$BUILD_NUMBER" ]; then echo "0"; else echo $$BUILD_NUMBER; fi)

INTERMEDIATE_DIR = $(TOP)/intermediate
TARGET_DIR  = $(TOP)/target/$(BUILD_CONFIGURATION)
INSTALLBUILDER_SUPPORT_DIR = $(TOP)/omi/Unix/installbuilder/datafiles

DATAFILES = Psrp.data

OUTPUTFILE_LINE = --OUTPUTFILE=psrp-$(PSRP_VERSION)-$(PSRP_PATCH_LEVEL).universal.$(PF_ARCH)

all:
	sudo rm -rf $(TOP)/target $(TOP)/intermediate
	mkdir -p $(TARGET_DIR) $(INTERMEDIATE_DIR)

ifeq ($(BUILD_RPM),1)
	@echo "========================= Make PSRP installer RPM"
	sudo rm -rf $(INTERMEDIATE_DIR)/staging
	python $(TOP)/pal/installer/InstallBuilder/installbuilder.py \
		--BASE_DIR=$(TOP) \
		--TARGET_DIR=$(TARGET_DIR) \
		--INTERMEDIATE_DIR=$(INTERMEDIATE_DIR) \
		--STAGING_DIR=$(INTERMEDIATE_DIR)/staging \
		--PF=$(PF) \
		--PFMAJOR=$(PF_MAJOR) \
		--PFMINOR=$(PF_MINOR) \
		--PFARCH=$(PF_ARCH) \
		--PFDISTRO=$(PF_DISTRO) \
		--VERSION=$(PSRP_VERSION) \
		--RELEASE=$(PSRP_PATCH_LEVEL) \
		$(OUTPUTFILE_LINE) \
		--DATAFILE_PATH=datafiles \
		$(DATAFILES) \
		$(INSTALLBUILDER_SUPPORT_DIR)/Linux_RPM.data
endif
ifeq ($(BUILD_DPKG),1)
	@echo "========================= Make PSRP installer DPKG"
	sudo rm -rf $(INTERMEDIATE_DIR)/staging
	python $(TOP)/pal/installer/InstallBuilder/installbuilder.py \
		--BASE_DIR=$(TOP) \
		--TARGET_DIR=$(TARGET_DIR) \
		--INTERMEDIATE_DIR=$(INTERMEDIATE_DIR) \
		--STAGING_DIR=$(INTERMEDIATE_DIR)/staging \
		--PF=$(PF) \
		--PFMAJOR=$(PF_MAJOR) \
		--PFMINOR=$(PF_MINOR) \
		--PFARCH=$(PF_ARCH) \
		--PFDISTRO=$(PF_DISTRO) \
		--VERSION=$(PSRP_VERSION) \
		--RELEASE=$(PSRP_PATCH_LEVEL) \
		$(OUTPUTFILE_LINE) \
		--DATAFILE_PATH=datafiles \
		$(DATAFILES) \
		$(INSTALLBUILDER_SUPPORT_DIR)/Linux_DPKG.data
endif
	rm -rf $(TARGET_DIR)/package_filename

	# Pick up the PSRP client library for distribution
	cp $(TOP)/src/libpsrpclient.* $(TARGET_DIR)

ifeq ($(PF),Darwin)
	# On mac we need to patch the dylib path for libmi.dylib, on Linux it is done in the linking
	install_name_tool -change '/opt/omi/lib/libmi.dylib' '@executable_path/libmi.dylib' $(TARGET_DIR)/libpsrpclient.dylib
endif

clean:
	sudo rm -rf $(TOP)/release $(TOP)/intermediate
