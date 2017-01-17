##
## This file is part of the coreboot project.
##
## Copyright (C) 2011 secunet Security Networks AG
## Copyright (C) 2014 Sage Electronic Engineering, LLC.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
##

#######################################################################
# misleadingly named, this is the coreboot version
export KERNELVERSION ?= "4.0$(KERNELREVISION)"

#######################################################################
# Test for coreboot toolchain (except when explicitely not requested)
ifneq ($(NOCOMPILE),1)
# only run if we're doing a build (not for tests, kconfig, ...)
_toolchain=$(shell $(CC_x86_32) -v 2>&1 |grep -q "gcc version .*coreboot toolchain" && echo coreboot)
ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
_toolchain=coreboot
endif
ifneq ($(_toolchain),coreboot)
$(error Please use the SageBIOS toolchain)
endif
endif

#######################################################################
# Basic component discovery
MAINBOARDDIR=$(call strip_quotes,$(CONFIG_MAINBOARD_DIR))
export MAINBOARDDIR

# Top level git directory for sagebios
GIT_DIR := ../.git
export GIT_DIR

## Final build results, which CBFSTOOL uses to create the final
## rom image file, are placed under $(objcbfs).
## These typically have suffixes .debug .elf .bin and .map
export objcbfs := $(obj)/cbfs/$(call strip_quotes,$(CONFIG_CBFS_PREFIX))

## Based on the active configuration, Makefile conditionally collects
## the required assembly includes and saves them in a file.
## Such files that do not have a clear one-to-one relation to a source
## file under src/ are placed and built under $(objgenerated)
export objgenerated := $(obj)/generated

#######################################################################
# root rule to resolve if in build mode (ie. configuration exists)
real-target: $(obj)/config.h coreboot
coreboot: build-dirs debug $(obj)/coreboot.rom final-toolcheck

#######################################################################
# our phony targets
PHONY+= clean-abuild coreboot lint lint-stable build-dirs final_warning

#######################################################################
# root source directories of coreboot
subdirs-y := src/lib src/console src/device src/ec src/southbridge src/soc
subdirs-y += src/northbridge src/superio src/drivers src/cpu src/vendorcode
subdirs-y += util/cbfstool util/sconfig util/nvramtool
subdirs-y += payloads
subdirs-y += src/arch/armv7 src/arch/x86
subdirs-y += src/mainboard/$(MAINBOARDDIR)

subdirs-y += site-local

#######################################################################
# Add source classes and their build options
classes-y := ramstage romstage driver cpu_microcode

ifneq ($(CONFIG_EXCLUDE_CLASSES_BOOTBLOCK),y)
classes-y += bootblock
endif
ifneq ($(CONFIG_EXCLUDE_CLASSES_SMM),y)
classes-y += smm smmstub
endif
ifneq ($(CONFIG_EXCLUDE_CLASSES_RMODULES),y)
classes-y += rmodules
endif


#######################################################################
# Helper functions for ramstage postprocess
spc :=
spc +=
$(spc) :=
$(spc) +=

# files-in-dir-recursive,dir,files
files-in-dir-recursive=$(filter $(1)%,$(2))

# parent-dir,dir/
parent-dir=$(dir $(if $(patsubst /%,,$(1)),,/)$(subst $( ),/,$(strip $(subst /, ,$(1)))))

# filters out exactly the directory specified
# filter-out-dir,dir_to_keep,dirs
filter-out-dir=$(filter-out $(1),$(2))

# filters out dir_to_keep and all its parents
# filter-out-dirs,dir_to_keep,dirs
filter-out-dirs=$(if $(filter-out ./ /,$(1)),$(call filter-out-dirs,$(call parent-dir,$(1)),$(call filter-out-dir,$(1),$(2))),$(call filter-out-dir,$(1),$(2)))

# dir-wildcards,dirs
dir-wildcards=$(addsuffix %,$(1))

# files-in-dir,dir,files
files-in-dir=$(filter-out $(call dir-wildcards,$(call filter-out-dirs,$(1),$(sort $(dir $(2))))),$(call files-in-dir-recursive,$(1),$(2)))

#######################################################################
# reduce command line length by linking the objects of each
# directory into an intermediate file
ramstage-postprocess=$(foreach d,$(sort $(dir $(1))), \
	$(eval $(d)ramstage.o: $(call files-in-dir,$(d),$(1)); $$(LD_ramstage) -o $$@ -r $$^ ) \
	$(eval ramstage-objs:=$(d)ramstage.o $(filter-out $(call files-in-dir,$(d),$(1)),$(ramstage-objs))))

romstage-c-ccopts:=-D__PRE_RAM__
romstage-S-ccopts:=-D__PRE_RAM__
ifeq ($(CONFIG_TRACE),y)
ramstage-c-ccopts:= -finstrument-functions
endif
ifeq ($(CONFIG_COVERAGE),y)
ramstage-c-ccopts+=-fprofile-arcs -ftest-coverage
endif

ifeq ($(CONFIG_USE_BLOBS),y)
forgetthis:=$(shell git submodule update --init --checkout 3rdparty)
endif

bootblock-c-ccopts:=-D__BOOT_BLOCK__ -D__PRE_RAM__
bootblock-S-ccopts:=-D__BOOT_BLOCK__ -D__PRE_RAM__

smmstub-c-ccopts:=-D__SMM__
smmstub-S-ccopts:=-D__SMM__
smm-c-ccopts:=-D__SMM__
smm-S-ccopts:=-D__SMM__

# SMM TSEG base is dynamic
ifneq ($(CONFIG_SMM_MODULES),y)
ifeq ($(CONFIG_SMM_TSEG),y)
smm-c-ccopts += -fpic
endif
endif

ramstage-c-deps:=$$(OPTION_TABLE_H)
romstage-c-deps:=$$(OPTION_TABLE_H)
bootblock-c-deps:=$$(OPTION_TABLE_H)
smm-c-deps:=$$(OPTION_TABLE_H)

#######################################################################
# Add handler to compile ACPI's ASL
define ramstage-objs_asl_template
$(obj)/$(1).ramstage.o: src/$(1).asl $(obj)/config.h
	@printf "    IASL       $$(subst $(top)/,,$$(@))\n"
	$(CC_ramstage) -x assembler-with-cpp -E -MMD -MT $$(@) -D__ACPI__ -P -include $(src)/include/kconfig.h -I$(obj) -I$(src) -I$(src)/include -I$(src)/arch/$(ARCHDIR-$(ARCH-ramstage-y))/include -I$(src)/mainboard/$(MAINBOARDDIR) $(CPPFLAGS_common) $$< -o $$(basename $$@).asl
	cd $$(dir $$@); $(IASL) -p $$(notdir $$@) -tc $$(notdir $$(basename $$@)).asl
	mv $$(basename $$@).hex $$(basename $$@).c
	$(CC_ramstage) $$(CFLAGS_ramstage) $$(CPPFLAGS_ramstage) $$(if $$(subst dsdt,,$$(basename $$(notdir $(1)))), -DAmlCode=AmlCode_$$(basename $$(notdir $(1)))) -c -o $$@ $$(basename $$@).c
	# keep %.o: %.c rule from catching the temporary .c file after a make clean
	mv $$(basename $$@).c $$(basename $$@).hex
endef

#######################################################################
# Parse plaintext cmos defaults into binary format
# arg1: source file
# arg2: binary file name
cbfs-files-processor-nvramtool= \
	$(eval $(2): $(1) $(src)/mainboard/$(MAINBOARDDIR)/cmos.layout | $(objutil)/nvramtool/nvramtool ; \
		printf "    CREATE     $(2) (from $(1))\n"; $(objutil)/nvramtool/nvramtool -y $(src)/mainboard/$(MAINBOARDDIR)/cmos.layout -D $(2).tmp -p $(1) && mv $(2).tmp $(2))

#######################################################################
# Link VSA binary to ELF-ish stage
# arg1: source file
# arg2: binary file name
cbfs-files-processor-vsa= \
	$(eval $(2): $(1) ; \
		printf "    CREATE     $(2) (from $(1))\n";  $(OBJCOPY_ramstage) --set-start 0x20 --adjust-vma 0x60000 -I binary -O elf32-i386 -B i386 $(1) $(2).tmp && $(LD_ramstage) -m elf_i386 -e 0x60020 --section-start .data=0x60000 $(2).tmp -o $(2))

#######################################################################
# Add handler for arbitrary files in CBFS
$(call add-special-class,cbfs-files)
cbfs-files-handler= \
		$(eval tmp-cbfs-method:=$(word 2, $(subst :, ,$($(2)-file)))) \
		$(eval $(2)-file:=$(call strip_quotes,$(word 1, $(subst :, ,$($(2)-file))))) \
		$(if $(wildcard $(1)$($(2)-file)), \
			$(eval tmp-cbfs-file:= $(wildcard $(1)$($(2)-file))), \
			$(eval tmp-cbfs-file:= $($(2)-file))) \
		$(if $(strip $($(2)-required)), \
			$(if $(wildcard $(tmp-cbfs-file)),, \
				$(info This build configuration requires $($(2)-required)) \
				$(eval FAILBUILD:=1) \
			)) \
		$(if $(tmp-cbfs-method), \
			$(eval tmp-old-cbfs-file:=$(tmp-cbfs-file)) \
			$(eval tmp-cbfs-file:=$(shell mkdir -p $(obj)/mainboard/$(MAINBOARDDIR); mktemp $(obj)/mainboard/$(MAINBOARDDIR)/cbfs-file.XXXXXX).out) \
			$(call cbfs-files-processor-$(tmp-cbfs-method),$(tmp-old-cbfs-file),$(tmp-cbfs-file))) \
		$(eval cbfs-files += $(tmp-cbfs-file)|$(2)|$($(2)-type)|$($(2)-compression)|$($(2)-position)) \
		$(eval $(2)-name:=) \
		$(eval $(2)-type:=) \
		$(eval $(2)-compression:=) \
		$(eval $(2)-position:=) \
		$(eval $(2)-required:=)

#######################################################################
# a variety of flags for our build
CBFS_COMPRESS_FLAG:=none
ifeq ($(CONFIG_COMPRESS_RAMSTAGE),y)
CBFS_COMPRESS_FLAG:=LZMA
endif

ifeq ($(CONFIG_COMPRESSED_PAYLOAD_LZMA),y)
CBFS_PAYLOAD_COMPRESS_FLAG:=-c LZMA
endif
ifeq ($(CONFIG_EXTRA_1ST_COMPRESSED_PAYLOAD_LZMA),y)
CBFS_EXTRA_1ST_PAYLOAD_COMPRESS_FLAG:=-c LZMA
endif
ifeq ($(CONFIG_EXTRA_2ND_COMPRESSED_PAYLOAD_LZMA),y)
CBFS_EXTRA_2ND_PAYLOAD_COMPRESS_FLAG:=-c LZMA
endif
ifeq ($(CONFIG_EXTRA_3RD_COMPRESSED_PAYLOAD_LZMA),y)
CBFS_EXTRA_3RD_PAYLOAD_COMPRESS_FLAG:=-c LZMA
endif
ifeq ($(CONFIG_EXTRA_4TH_COMPRESSED_PAYLOAD_LZMA),y)
CBFS_EXTRA_4TH_PAYLOAD_COMPRESS_FLAG:=-c LZMA
endif

ifneq ($(shell printf "%d" $(CONFIG_PAYLOAD_BASE)),0)
PAYLOAD_LOCATION := -b $(CONFIG_PAYLOAD_BASE)
endif
ifneq ($(shell printf "%d" $(CONFIG_EXTRA_1ST_PAYLOAD_BASE)),0)
PAYLOAD_EXTRA_1ST_LOCATION := -b $(CONFIG_EXTRA_1ST_PAYLOAD_BASE)
endif
ifneq ($(shell printf "%d" $(CONFIG_EXTRA_2ND_PAYLOAD_BASE)),0)
PAYLOAD_EXTRA_2ND_LOCATION := -b $(CONFIG_EXTRA_2ND_PAYLOAD_BASE)
endif
ifneq ($(shell printf "%d" $(CONFIG_EXTRA_3RD_PAYLOAD_BASE)),0)
PAYLOAD_EXTRA_3RD_LOCATION := -b $(CONFIG_EXTRA_3RD_PAYLOAD_BASE)
endif
ifneq ($(shell printf "%d" $(CONFIG_EXTRA_4TH_PAYLOAD_BASE)),0)
PAYLOAD_EXTRA_4TH_LOCATION := -b $(CONFIG_EXTRA_4TH_PAYLOAD_BASE)
endif


ifneq ($(CONFIG_LOCALVERSION),"")
COREBOOT_EXTRA_VERSION := -$(call strip_quotes,$(CONFIG_LOCALVERSION))
endif

CPPFLAGS_common := -Isrc -Isrc/include -I$(obj)
CPPFLAGS_common += -include $(src)/include/kconfig.h

CFLAGS_common += -Os -pipe -g -nostdinc
CFLAGS_common += -nostdlib -Wall -Wundef -Wstrict-prototypes -Wmissing-prototypes
CFLAGS_common += -Wwrite-strings -Wredundant-decls -Wno-trigraphs
CFLAGS_common += -Wstrict-aliasing -Wshadow
ifeq ($(CONFIG_WARNINGS_ARE_ERRORS),y)
CFLAGS_common += -Werror
endif
CFLAGS_common += -fno-common -ffreestanding -fno-builtin -fomit-frame-pointer

additional-dirs := $(objutil)/cbfstool $(objutil)/romcc $(objutil)/ifdtool \
		   $(objutil)/ifdfake $(objutil)/options $(objutil)/fletcher

#######################################################################
# generate build support files
$(obj)/build.h: .xcompile
	@printf "    GEN        build.h\n"
	rm -f $(obj)/build.h
	set -e ; \
	export KERNELVERSION="\"$(KERNELVERSION)\"" ; \
	export COREBOOT_EXTRA_VERSION="$(COREBOOT_EXTRA_VERSION)" ; \
	export CC="$(CC)" ; \
	export AS="$(AS)" ; \
	export LD="$(LD)" ; \
	export ROUND_TIME="$(ROUND_TIME)" ; \
	util/genbuild_h/genbuild_h.sh > $(obj)/build.ht
	mv $(obj)/build.ht $(obj)/build.h

$(obj)/ldoptions: $(obj)/config.h
	awk '/^#define ([^"])* ([^"])*$$/ {gsub("\\r","",$$3); print $$2 " = " $$3 ";";}' $< > $@

build-dirs:
	mkdir -p $(objcbfs) $(objgenerated)

#######################################################################
# Build the tools
CBFSTOOL:=$(obj)/cbfstool
RMODTOOL:=$(obj)/rmodtool

$(CBFSTOOL): $(objutil)/cbfstool/cbfstool
	cp $< $@

$(RMODTOOL): $(objutil)/cbfstool/rmodtool
	cp $< $@

_WINCHECK=$(shell uname -o 2> /dev/null)
STACK=
ifeq ($(_WINCHECK),Msys)
	STACK=-Wl,--stack,16384000
endif
ifeq ($(_WINCHECK),Cygwin)
	STACK=-Wl,--stack,16384000
endif

# this allows ccache to prepend itself
# (ccache handling happens first)
ROMCC_BIN= $(objutil)/romcc/romcc
ROMCC?=$(ROMCC_BIN)
$(ROMCC_BIN): $(top)/util/romcc/romcc.c
	@printf "    HOSTCC     $(subst $(obj)/,,$(@)) (this may take a while)\n"
	@# Note: Adding -O2 here might cause problems. For details see:
	@# http://www.coreboot.org/pipermail/coreboot/2010-February/055825.html
	$(HOSTCC) $(HOSTCFLAGS) $(STACK) -Wall -o $@ $<

IFDTOOL:=$(objutil)/ifdtool/ifdtool
$(IFDTOOL): $(top)/util/ifdtool/ifdtool.c
	@printf "    HOSTCC     $(subst $(obj)/,,$(@))\n"
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

IFDFAKE:=$(objutil)/ifdfake/ifdfake
$(IFDFAKE): $(top)/util/ifdfake/ifdfake.c
	@printf "    HOSTCC     $(subst $(obj)/,,$(@))\n"
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

FLETCHER:=$(objutil)/fletcher/fletcher
$(FLETCHER): $(top)/util/fletcher/fletcher.c
	@printf "    HOSTCC     $(subst $(obj)/,,$(@))\n"
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

#######################################################################
# needed objects that every mainboard uses
# Creation of these is architecture and mainboard independent
$(obj)/mainboard/$(MAINBOARDDIR)/static.c: $(src)/mainboard/$(MAINBOARDDIR)/devicetree.cb  $(objutil)/sconfig/sconfig
	@printf "    SCONFIG    $(subst $(src)/,,$(<))\n"
	mkdir -p $(obj)/mainboard/$(MAINBOARDDIR)
	$(objutil)/sconfig/sconfig $(MAINBOARDDIR) $(obj)/mainboard/$(MAINBOARDDIR)

ramstage-y+=$(obj)/mainboard/$(MAINBOARDDIR)/static.c
romstage-y+=$(obj)/mainboard/$(MAINBOARDDIR)/static.c

$(objutil)/%.o: $(objutil)/%.c
	@printf "    HOSTCC     $(subst $(objutil)/,,$(@))\n"
	$(HOSTCC) -I$(subst $(objutil)/,util/,$(dir $<)) -I$(dir $<) $(HOSTCFLAGS) -c -o $@ $<

$(obj)/%.ramstage.o $(abspath $(obj))/%.ramstage.o: $(obj)/%.c $(obj)/config.h $(OPTION_TABLE_H)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC_ramstage) -MMD $(CFLAGS_ramstage) $(CPPFLAGS_ramstage) -c -o $@ $<

$(obj)/%.romstage.o $(abspath $(obj))/%.romstage.o: $(obj)/%.c $(obj)/config.h $(OPTION_TABLE_H)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC_romstage) -MMD -D__PRE_RAM__ $(CFLAGS_romstage) $(CPPFLAGS_romstage) -c -o $@ $<

$(obj)/%.bootblock.o $(abspath $(obj))/%.bootblock.o: $(obj)/%.c $(obj)/config.h $(OPTION_TABLE_H)
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC_bootblock) -MMD $(bootblock-c-ccopts) $(CFLAGS_bootblock) $(CPPFLAGS_bootblock) -c -o $@ $<

#######################################################################
# Clean up rules
clean-abuild:
	rm -rf coreboot-builds

clean-for-update-target:
	rm -f $(obj)/ramstage* $(obj)/coreboot.romstage $(obj)/coreboot.pre* $(obj)/coreboot.bootblock $(obj)/coreboot.a
	rm -rf $(obj)/bootblock* $(obj)/romstage* $(obj)/location.*
	rm -f $(obj)/option_table.* $(obj)/crt0.S $(obj)/ldscript
	rm -f $(obj)/mainboard/$(MAINBOARDDIR)/static.c $(obj)/mainboard/$(MAINBOARDDIR)/config.py $(obj)/mainboard/$(MAINBOARDDIR)/static.dot
	rm -f $(obj)/mainboard/$(MAINBOARDDIR)/crt0.s $(obj)/mainboard/$(MAINBOARDDIR)/crt0.disasm
	rm -f $(obj)/mainboard/$(MAINBOARDDIR)/romstage.inc
	rm -f $(obj)/mainboard/$(MAINBOARDDIR)/bootblock.* $(obj)/mainboard/$(MAINBOARDDIR)/dsdt.*
	rm -f $(obj)/cpu/x86/smm/smm_bin.c $(obj)/cpu/x86/smm/smm.* $(obj)/cpu/x86/smm/smm

clean-target:
	rm -f $(obj)/coreboot*

#######################################################################
# Development utilities
printcrt0s:
	@echo crt0s=$(crt0s)
	@echo ldscripts=$(ldscripts)

update:
	dongle.py -c /dev/term/1 $(obj)/coreboot.rom EOF

lint lint-stable:
	FAILED=0; LINTLOG=`mktemp .tmpconfig.lintXXXXX`; \
	for script in util/lint/$@-*; do \
		echo; echo `basename $$script`; \
		grep "^# DESCR:" $$script | sed "s,.*DESCR: *,," ; \
		echo ========; \
		$$script > $$LINTLOG; \
		if [ `cat $$LINTLOG | wc -l` -eq 0 ]; then \
			printf "success\n\n"; \
		else \
			echo test failed: ; \
			cat $$LINTLOG; \
			rm -f $$LINTLOG; \
			FAILED=$$(( $$FAILED + 1 )); \
		fi; \
		echo ========; \
	done; \
	test $$FAILED -eq 0 || { echo "ERROR: $$FAILED test(s) failed." &&  exit 1; }; \
	rm -f $$LINTLOG

gitconfig:
	mkdir -p $(GIT_DIR)/hooks
	for hook in commit-msg pre-commit ; do                       \
		if [ util/gitconfig/$$hook -nt $(GIT_DIR)/hooks/$$hook -o  \
		! -x $(GIT_DIR)/hooks/$$hook ]; then			     \
			cp util/gitconfig/$$hook $(GIT_DIR)/hooks/$$hook;  \
			chmod +x $(GIT_DIR)/hooks/$$hook;		     \
		fi;						     \
	done
	git config remote.origin.push HEAD:refs/for/master
	(git config --global user.name >/dev/null && git config --global user.email >/dev/null) || (printf 'Please configure your name and email in git:\n\n git config --global user.name "Your Name Comes Here"\n git config --global user.email your.email@example.com\n'; exit 1)

crossgcc: crossgcc-i386 crossgcc-arm

.PHONY: crossgcc-i386 crossgcc-arm
crossgcc-i386: clean-for-update
	$(MAKE) -C util/crossgcc build-i386-without-gdb

crossgcc-arm: clean-for-update
	$(MAKE) -C util/crossgcc build-armv7a-without-gdb

crosstools: crosstools-i386

.PHONY: crosstools-i386 crosstools-arm
crosstools-i386: clean-for-update
	$(MAKE) -C util/crossgcc build-i386

crosstools-arm: clean-for-update
	$(MAKE) -C util/crossgcc build-armv7a

crossgcc-clean: clean-for-update
	$(MAKE) -C util/crossgcc clean

tools: $(objutil)/kconfig/conf $(objutil)/cbfstool/cbfstool $(objutil)/cbfstool/rmodtool $(objutil)/nvramtool/nvramtool $(objutil)/romcc/romcc $(objutil)/sconfig/sconfig

###########################################################################
# Common recipes for all stages
###########################################################################

# find-substr is required for stages like romstage_null and romstage_xip to
# eliminate the _* part of the string
find-substr = $(word 1,$(subst _, ,$(1)))

# find-class is used to identify the class from the name of the stage
# The input to this macro can be something like romstage.x or romstage.x.y
# find-class recursively strips off the suffixes to extract the exact class name
# e.g.: if romstage.x is provided to find-class, it will remove .x and return romstage
# if romstage.x.y is provided, it will first remove .y, call find-class with romstage.x
# and remove .x the next time and finally return romstage
find-class = $(if $(filter $(1),$(basename $(1))),$(if $(CC_$(1)), $(1), $(call find-substr,$(1))),$(call find-class,$(basename $(1))))

$(objcbfs)/%.bin: $(objcbfs)/%.elf
	$(eval class := $(call find-class,$(@F)))
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	$(OBJCOPY_$(class)) -O binary $< $@

$(objcbfs)/%.elf: $(objcbfs)/%.debug
	$(eval class := $(call find-class,$(@F)))
	@printf "    OBJCOPY    $(subst $(obj)/,,$(@))\n"
	cp $< $@.tmp
	$(NM_$(class)) -n $@.tmp | sort > $(basename $@).map
	$(OBJCOPY_$(class)) --strip-debug $@.tmp
	$(OBJCOPY_$(class)) --add-gnu-debuglink=$< $@.tmp
	mv $@.tmp $@

###########################################################################
# Build the final rom image
###########################################################################

ifeq ($(CONFIG_PAYLOAD_ELF),y)
COREBOOT_ROM_DEPENDENCIES+=$(call strip_quotes,$(CONFIG_PAYLOAD_FILE))
endif

extract_nth=$(word $(1), $(subst |, ,$(2)))

ifneq ($(CONFIG_UPDATE_IMAGE),y)
# Use ';' instead of '&&' to join the cbfstool commands in this macro - the
# open source package creation process depends on being able to split multiple
# commands on a single line on the ';'.
prebuild-files = \
	       $(foreach file,$(cbfs-files), \
	       $(CBFSTOOL) $@.tmp \
	       add$(if $(filter stage,$(call extract_nth,3,$(file))),-stage)$(if $(filter payload,$(call extract_nth,3,$(file))),-payload) \
	       -f $(call extract_nth,1,$(file)) \
	       -n $(call extract_nth,2,$(file)) $(if $(filter-out stage,$(call extract_nth,3,$(file))),-t $(call extract_nth,3,$(file))) \
	       $(if $(call extract_nth,4,$(file)),-b $(call extract_nth,4,$(file)));)
prebuilt-files = $(foreach file,$(cbfs-files), $(call extract_nth,1,$(file)))

$(obj)/coreboot.pre1: $(objcbfs)/bootblock.bin $$(prebuilt-files) $(CBFSTOOL) $$(cpu_ucode_cbfs_file)
	$(CBFSTOOL) $@.tmp create -s $(CONFIG_COREBOOT_ROMSIZE_KB)K \
	-B $(objcbfs)/bootblock.bin -a 64 \
	$(CBFSTOOL_PRE1_OPTS)
	$(prebuild-files)
	$(call add-cpu-microcode-to-cbfs,$@.tmp)
	mv $@.tmp $@
else
.PHONY: $(obj)/coreboot.pre1
$(obj)/coreboot.pre1: $(CBFSTOOL)
	mv $(obj)/coreboot.rom $@
endif

cbfs_user_additions-$(CONFIG_BOOTSPLASH) += $(call strip_quotes,$(CONFIG_BOOTSPLASH_FILE))
ifneq (,$(call strip_quotes,$(CONFIG_BOOTSPLASH_ADDRESS)))
bootsplash.jpg-position := -b $(call strip_quotes,$(CONFIG_BOOTSPLASH_ADDRESS))
endif

ifeq (y,$(CONFIG_VGA_BIOS))
ifeq ($(findstring payloads,$(CONFIG_VGA_BIOS_FILE)),)
cbfs_user_additions-$(CONFIG_VGA_BIOS) += $(call strip_quotes,$(CONFIG_VGA_BIOS_FILE))
else
COREBOOT_ROM_DEPENDENCIES+=$(call strip_quotes,$(CONFIG_VGA_BIOS_FILE))
endif
ifeq (y,$(CONFIG_VGA_BIOS_PUT_IN_VGAROMS))
vbios_cbfs_name := "vgaroms/$(CONFIG_MAINBOARD_PART_NUMBER)_vbios.rom"
else
vbios_cbfs_name := "pci$(CONFIG_VGA_BIOS_ID).rom"
endif
ifneq (,$(call strip_quotes,$(CONFIG_VGA_BIOS_ADDRESS)))
$(vbios_cbfs_name)-position := -b $(call strip_quotes,$(CONFIG_VGA_BIOS_ADDRESS))
endif
endif

ifeq ($(CONFIG_PAYLOAD_LINUX),y)
ifneq ($(strip $(call strip_quotes,$(CONFIG_LINUX_COMMAND_LINE))),)
      ADDITIONAL_PAYLOAD_CONFIG+=-C $(CONFIG_LINUX_COMMAND_LINE)
endif
ifneq ($(strip $(call strip_quotes,$(CONFIG_LINUX_INITRD))),)
      ADDITIONAL_PAYLOAD_CONFIG+=-I $(CONFIG_LINUX_INITRD)
endif
endif

ifeq ($(CONFIG_HAVE_REFCODE_BLOB),y)
REFCODE_BLOB=$(obj)/refcode.rmod
$(REFCODE_BLOB): $(RMODTOOL)
	$(RMODTOOL) -i $(CONFIG_REFCODE_BLOB_FILE) -o $@
endif

cbfs_user_additions-$(CONFIG_OPTION_ROM) += $(call strip_quotes,$(CONFIG_OPTION_ROM_FILE_IN))
stripped_orom = $(call strip_quotes,$(CONFIG_OPTION_ROM_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_OPTION_ROM_ADDRESS)))
$(stripped_orom)-position := -b $(call strip_quotes,$(CONFIG_OPTION_ROM_ADDRESS))
endif

ifneq ("path/filename",$(CONFIG_CBFS_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_FILE) += $(call strip_quotes,$(CONFIG_CBFS_FILE_FILE_IN))
stripped_cbfs_file = $(call strip_quotes,$(CONFIG_CBFS_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_FILE_ADDRESS)))
$(stripped_cbfs_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_FILE_ADDRESS))
endif
endif

ifneq ("path/filename",$(CONFIG_CBFS_2ND_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_2ND_FILE) += $(call strip_quotes,$(CONFIG_CBFS_2ND_FILE_FILE_IN))
stripped_cbfs_2nd_file = $(call strip_quotes,$(CONFIG_CBFS_2ND_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_2ND_FILE_ADDRESS)))
$(stripped_cbfs_2nd_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_2ND_FILE_ADDRESS))
endif
endif

ifneq ("path/filename",$(CONFIG_CBFS_3RD_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_3RD_FILE) += $(call strip_quotes,$(CONFIG_CBFS_3RD_FILE_FILE_IN))
stripped_cbfs_3rd_file = $(call strip_quotes,$(CONFIG_CBFS_3RD_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_3RD_FILE_ADDRESS)))
$(stripped_cbfs_3rd_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_3RD_FILE_ADDRESS))
endif
endif

ifneq ("path/filename",$(CONFIG_CBFS_4TH_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_4TH_FILE) += $(call strip_quotes,$(CONFIG_CBFS_4TH_FILE_FILE_IN))
stripped_cbfs_4th_file = $(call strip_quotes,$(CONFIG_CBFS_4TH_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_4TH_FILE_ADDRESS)))
$(stripped_cbfs_4th_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_4TH_FILE_ADDRESS))
endif
endif

ifneq ("path/filename",$(CONFIG_CBFS_5TH_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_5TH_FILE) += $(call strip_quotes,$(CONFIG_CBFS_5TH_FILE_FILE_IN))
stripped_cbfs_5th_file = $(call strip_quotes,$(CONFIG_CBFS_5TH_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_5TH_FILE_ADDRESS)))
$(stripped_cbfs_5th_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_5TH_FILE_ADDRESS))
endif
endif

ifneq ("path/filename",$(CONFIG_CBFS_6TH_FILE_FILE_IN))
cbfs_user_additions-$(CONFIG_CBFS_6TH_FILE) += $(call strip_quotes,$(CONFIG_CBFS_6TH_FILE_FILE_IN))
stripped_cbfs_6th_file = $(call strip_quotes,$(CONFIG_CBFS_6TH_FILE_FILE_OUT))
ifneq (,$(call strip_quotes,$(CONFIG_CBFS_6TH_FILE_ADDRESS)))
$(stripped_cbfs_6th_file)-position := -b $(call strip_quotes,$(CONFIG_CBFS_6TH_FILE_ADDRESS))
endif
endif

cbfs_user_additions-$(CONFIG_SEABIOS_ADD_BOOTORDER) += $(call strip_quotes,$(CONFIG_SEABIOS_BOOTORDER_FILE))

$(cbfs_user_additions-y):
	echo                                                                              ; \
	echo "###########################################################################"; \
	echo "# ERROR: The user requested CBFS file does not exist"                       ; \
	echo "# 	   $@"                                                                ; \
	echo "# Please update your build configuration to correct the file name/path."    ; \
	if [ "$@" = "$(call strip_quotes,$(CONFIG_VGA_BIOS_FILE))" ]; then                  \
	echo "# -------------------------------------------------------------------------"; \
	echo "# For instructions on downloading a vbios please check Sage's website."     ; \
	echo "# http://www.se-eng.com/forum/viewtopic.php?f=3&t=54"                       ; \
	fi                                                                                ; \
	echo "###########################################################################"; \
	echo                                                                              ;
	__CBFS_USER_ERROR___END_BUILD__

# Specify the components that make up the coreboot.rom image
COREBOOT_ROM_COMPONENTS += $(call strip_quotes,$(COREBOOT_ROM_DEPENDENCIES))
COREBOOT_ROM_COMPONENTS += $(cbfs_user_additions-y)
COREBOOT_ROM_COMPONENTS += $(obj)/coreboot.pre
ifneq ($(CONFIG_EXCLUDE_RAMSTAGE),y)
COREBOOT_ROM_COMPONENTS += $(objcbfs)/ramstage.elf
endif

$(obj)/coreboot.rom: $(CBFSTOOL) $$(COREBOOT_ROM_COMPONENTS) $$(INTERMEDIATE) $$(VBOOT_STUB) $(REFCODE_BLOB)
	@printf "    CBFS       $(subst $(obj)/,,$(@))\n"
	cp $(obj)/coreboot.pre $@.tmp
ifneq ($(CONFIG_EXCLUDE_RAMSTAGE),y)
	$(CBFSTOOL) $@.tmp add-stage -f $(objcbfs)/ramstage.elf -n $(CONFIG_CBFS_PREFIX)/ramstage -c $(CBFS_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_BOOTSPLASH),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_BOOTSPLASH_FILE)) -n bootsplash.jpg $(bootsplash.jpg-position) -t bootsplash
endif
ifeq ($(CONFIG_PAYLOAD_NONE),y)
	echo                                                                              ; \
	echo "###########################################################################"; \
	echo "# WARNING: The user did not specify a payload for this build."              ; \
	echo "# If this is incorrect, please use the Kconfig Tool in the SageEDK to"      ; \
	echo "# configure your build for the desired payload file."                       ; \
	echo "###########################################################################";
	echo                                                                              ;
	@printf "    PAYLOAD    none (as specified by user)\n"
endif
ifneq ($(CONFIG_PAYLOAD_FILE),)
	@printf "    PAYLOAD    $(CONFIG_PAYLOAD_FILE) -n $(CONFIG_CBFS_PREFIX)/payload $(PAYLOAD_LOCATION) $(CBFS_PAYLOAD_COMPRESS_FLAG) $(ADDITIONAL_PAYLOAD_CONFIG)\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_PAYLOAD_FILE) -n $(CONFIG_CBFS_PREFIX)/payload $(PAYLOAD_LOCATION) $(CBFS_PAYLOAD_COMPRESS_FLAG) $(ADDITIONAL_PAYLOAD_CONFIG)
endif
ifeq ($(CONFIG_EXTRA_1ST_PAYLOAD),y)
	@printf "    PAYLOAD    $(CONFIG_EXTRA_1ST_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_1ST_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_1ST_LOCATION) $(CBFS_EXTRA_1ST_PAYLOAD_COMPRESS_FLAG)\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_EXTRA_1ST_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_1ST_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_1ST_LOCATION) $(CBFS_EXTRA_1ST_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_EXTRA_2ND_PAYLOAD),y)
	@printf "    PAYLOAD    $(CONFIG_EXTRA_2ND_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_2ND_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_2ND_LOCATION) $(CBFS_EXTRA_2ND_PAYLOAD_COMPRESS_FLAG)\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_EXTRA_2ND_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_2ND_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_2ND_LOCATION) $(CBFS_EXTRA_2ND_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_EXTRA_3RD_PAYLOAD),y)
	@printf "    PAYLOAD    $(CONFIG_EXTRA_3RD_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_3RD_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_3RD_LOCATION) $(CBFS_EXTRA_3RD_PAYLOAD_COMPRESS_FLAG)\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_EXTRA_3RD_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_3RD_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_3RD_LOCATION) $(CBFS_EXTRA_3RD_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_EXTRA_4TH_PAYLOAD),y)
	@printf "    PAYLOAD    $(CONFIG_EXTRA_4TH_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_4TH_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_4TH_LOCATION) $(CBFS_EXTRA_4TH_PAYLOAD_COMPRESS_FLAG)\n"
	$(CBFSTOOL) $@.tmp add-payload -f $(CONFIG_EXTRA_4TH_PAYLOAD_FILE) -n img/$(CONFIG_EXTRA_4TH_PAYLOAD_CBFS_NAME) $(PAYLOAD_EXTRA_4TH_LOCATION) $(CBFS_EXTRA_4TH_PAYLOAD_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_INCLUDE_CONFIG_FILE),y)
	@printf "    CONFIG     $(DOTCONFIG)\n"
	if [ -f $(DOTCONFIG) ]; then \
	sed -e '/^#/d' -e '/^ *$$/d' $(DOTCONFIG) >> $(obj)/config.tmp ; \
	$(CBFSTOOL) $@.tmp add -f $(obj)/config.tmp -n config -t raw; rm -f $(obj)/config.tmp ; fi
endif
ifeq ($(CONFIG_VBOOT_VERIFY_FIRMWARE),y)
	$(CBFSTOOL) $@.tmp add-stage -f $(VBOOT_STUB) -n $(CONFIG_CBFS_PREFIX)/vboot -c $(CBFS_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_HAVE_REFCODE_BLOB),y)
	$(CBFSTOOL) $@.tmp add-stage -f $(REFCODE_BLOB) -n $(CONFIG_CBFS_PREFIX)/refcode -c $(CBFS_COMPRESS_FLAG)
endif
ifeq ($(CONFIG_VGA_BIOS),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_VGA_BIOS_FILE)) -n $(vbios_cbfs_name) $($(vbios_cbfs_name)-position) -t optionrom
endif
ifeq ($(CONFIG_OPTION_ROM),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_OPTION_ROM_FILE_IN)) -n $(stripped_orom) $($(stripped_orom)-position) -t optionrom
endif
ifeq ($(CONFIG_CBFS_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_FILE_FILE_IN)) -n $(stripped_cbfs_file) $($(stripped_cbfs_file)-position) -t raw
endif
ifeq ($(CONFIG_CBFS_2ND_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_2ND_FILE_FILE_IN)) -n $(stripped_cbfs_2nd_file) $($(stripped_cbfs_2nd_file)-position) -t raw
endif
ifeq ($(CONFIG_CBFS_3RD_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_3RD_FILE_FILE_IN)) -n $(stripped_cbfs_3rd_file) $($(stripped_cbfs_3rd_file)-position) -t raw
endif
ifeq ($(CONFIG_CBFS_4TH_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_4TH_FILE_FILE_IN)) -n $(stripped_cbfs_4th_file) $($(stripped_cbfs_4th_file)-position) -t raw
endif
ifeq ($(CONFIG_CBFS_5TH_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_5TH_FILE_FILE_IN)) -n $(stripped_cbfs_5th_file) $($(stripped_cbfs_5th_file)-position) -t raw
endif
ifeq ($(CONFIG_CBFS_6TH_FILE),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_CBFS_6TH_FILE_FILE_IN)) -n $(stripped_cbfs_6th_file) $($(stripped_cbfs_6th_file)-position) -t raw
endif
ifeq ($(CONFIG_SEABIOS_ADD_BOOTORDER),y)
	$(CBFSTOOL) $@.tmp add -f $(call strip_quotes,$(CONFIG_SEABIOS_BOOTORDER_FILE)) -n bootorder -t raw
endif
ifeq ($(CONFIG_ADD_BOOT_MENU_WAIT),y)
	payloads/seabios/scripts/encodeint.py build/boot-menu-wait $(CONFIG_BOOT_MENU_WAIT_TIME)
	$(CBFSTOOL) $@.tmp add -f build/boot-menu-wait -n etc/boot-menu-wait -t raw
endif
ifeq ($(CONFIG_CPU_INTEL_FIRMWARE_INTERFACE_TABLE),y)
ifeq ($(CONFIG_CPU_MICROCODE_ADDED_DURING_BUILD),y)
	@printf "    UPDATE-FIT \n"
	$(CBFSTOOL) $@.tmp update-fit -n cpu_microcode_blob.bin -x $(CONFIG_CPU_INTEL_NUM_FIT_ENTRIES)
endif
endif
	mv $@.tmp $@
	@printf "    CBFSPRINT  $(subst $(obj)/,,$(@))\n\n"
	$(CBFSTOOL) $@ print
	@printf "\nBuilt $(CONFIG_MAINBOARD_DIR) mainboard.\n"

ifeq ($(CONFIG_ARCH_ROMSTAGE_ARMV7),y)
ROMSTAGE_ELF := romstage.elf
endif
ifeq ($(CONFIG_ARCH_ROMSTAGE_X86_32),y)
ROMSTAGE_ELF := romstage_xip.elf
endif

$(obj)/coreboot.pre: $(objcbfs)/$(ROMSTAGE_ELF) $(obj)/coreboot.pre1 $(CBFSTOOL)
	@printf "    CBFS       $(subst $(obj)/,,$(@))\n"
	cp $(obj)/coreboot.pre1 $@.tmp
	$(CBFSTOOL) $@.tmp add-stage \
		-f $(objcbfs)/$(ROMSTAGE_ELF) \
		-n $(CONFIG_CBFS_PREFIX)/romstage -c none \
		$(CBFSTOOL_PRE_OPTS)
	mv $@.tmp $@

# The coreboot/utils generate dependency files for the SageBIOS build, we
# might as well include them in the Makefile so make is able to use them when
# trying to determine which object files need to be rebuilt.
-include $(DEPS)
