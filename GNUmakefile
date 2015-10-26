TOP = ../omi/Unix
PSNATIVEHOST = ../monad-native/host

include $(TOP)/config.mak

CSHLIBRARY = psrpomiprov

DEFINES = HOOK_BUILD

SOURCES = Shell.c module.c schema.c xpress.c BufferManipulation.c

INCLUDES = $(TOP) $(TOP)/common $(PSNATIVEHOST)

LIBRARIES = miapi omi_error wsman xmlserializer protocol sock provmgr wql base pal pshost stdc++ icuuc

include $(TOP)/mak/rules.mak

gen:
	$(BINDIR)/omigen  -C $(TOP)/share/networkschema/CIM_Schema.mof schema.mof Shell -e Stream -e CommandState -e EnvironmentVariable

gen2:
	$(BINDIR)/omigen  -C $(BINDIR)/omigen -C $(TOP)/cimschema/CIM_Schema.mof schema.mof Shell

REGFILE=$(CSHLIBRARY).reg

reg:
	$(BINDIR)/omireg $(TARGET) -n interop
	cp $(REGFILE) "$(CONFIG_SYSCONFDIR)/omiregister/interop/$(REGFILE)"

unreg:
	rm -f $(REGFILE) "$(CONFIG_SYSCONFDIR)/omiregister/interop/$(REGFILE)"

ei:
	$(BINDIR)/omicli ei interop Shell

gi:
	$(BINDIR)/omicli gi interop { Shell InstanceID "2FDB5542-5896-45D5-9BE9-DC04430AAABE" }
