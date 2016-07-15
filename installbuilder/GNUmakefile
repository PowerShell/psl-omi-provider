TOP = $(shell cd ../omi/Unix ; pwd)
include $(TOP)/../../../pal/build/config.mak
include $(TOP)/config.mak

DISTRO_TYPE = $(PF)
ifeq ($(PF),Linux)
DISTRO_TYPE = $(PF)_$(PF_DISTRO)
endif

DATAFILES = Psrp.data

OUTPUTFILE_LINE = --OUTPUTFILE=psrp-$(CONFIG_VERSION)-$(CONFIG_PATCH_LEVEL).universal.$(PF_ARCH)

all:
ifneq ($(PF),Linux)
	@echo "========================= Make OMI installer $(PF_DISTRO)"
	sudo rm -rf $(OUTPUTDIR)/intermediate/staging
	mkdir -p $(OUTPUTDIR)/release $(OUTPUTDIR)/intermediate
	python $(TOP)/../../../pal/installer/InstallBuilder/installbuilder.py \
		--BASE_DIR=$(OUTPUTDIR) \
		--TARGET_DIR=$(OUTPUTDIR)/release \
		--INTERMEDIATE_DIR=$(OUTPUTDIR)/intermediate \
		--STAGING_DIR=$(OUTPUTDIR)/intermediate/staging \
		--PF=$(PF) \
		--PFMAJOR=$(PF_MAJOR) \
		--PFMINOR=$(PF_MINOR) \
		--PFARCH=$(PF_ARCH) \
		--PFDISTRO=$(PF_DISTRO) \
		--VERSION=$(CONFIG_VERSION) \
		--RELEASE=$(CONFIG_PATCH_LEVEL) \
		$(OUTPUTFILE_LINE) \
		--DATAFILE_PATH=datafiles \
		$(DATAFILES)
else
 ifeq ($(BUILD_RPM),1)
	@echo "========================= Make OMI installer RPM"
	sudo rm -rf $(OUTPUTDIR)/intermediate/staging
	mkdir -p $(OUTPUTDIR)/release $(OUTPUTDIR)/intermediate
	python $(TOP)/../../../pal/installer/InstallBuilder/installbuilder.py \
		--BASE_DIR=$(OUTPUTDIR) \
		--TARGET_DIR=$(OUTPUTDIR)/release \
		--INTERMEDIATE_DIR=$(OUTPUTDIR)/intermediate \
		--STAGING_DIR=$(OUTPUTDIR)/intermediate/staging \
		--PF=$(PF) \
		--PFMAJOR=$(PF_MAJOR) \
		--PFMINOR=$(PF_MINOR) \
		--PFARCH=$(PF_ARCH) \
		--PFDISTRO=$(PF_DISTRO) \
		--VERSION=$(CONFIG_VERSION) \
		--RELEASE=$(CONFIG_PATCH_LEVEL) \
		$(OUTPUTFILE_LINE) \
		--DATAFILE_PATH=datafiles \
		$(DATAFILES) \
		$(TOP)/installbuilder/datafiles/Linux_RPM.data
 endif
 ifeq ($(BUILD_DPKG),1)
	echo "========================= Make OMI installer DPKG"
	sudo rm -rf $(OUTPUTDIR)/intermediate/staging
	mkdir -p $(OUTPUTDIR)/release $(OUTPUTDIR)/intermediate
	python $(TOP)/../../../pal/installer/InstallBuilder/installbuilder.py \
		--BASE_DIR=$(OUTPUTDIR) \
		--TARGET_DIR=$(OUTPUTDIR)/release \
		--INTERMEDIATE_DIR=$(OUTPUTDIR)/intermediate \
		--STAGING_DIR=$(OUTPUTDIR)/intermediate/staging \
		--PF=$(PF) \
		--PFMAJOR=$(PF_MAJOR) \
		--PFMINOR=$(PF_MINOR) \
		--PFARCH=$(PF_ARCH) \
		--PFDISTRO=$(PF_DISTRO) \
		--VERSION=$(CONFIG_VERSION) \
		--RELEASE=$(CONFIG_PATCH_LEVEL) \
		$(OUTPUTFILE_LINE) \
		--DATAFILE_PATH=datafiles \
		$(DATAFILES) \
		$(TOP)/installbuilder/datafiles/Linux_DPKG.data
 endif
endif
clean:

#	sudo rm -rf $(OUTPUTDIR)/intermediate
#	sudo rm -rf $(OUTPUTDIR)/release
