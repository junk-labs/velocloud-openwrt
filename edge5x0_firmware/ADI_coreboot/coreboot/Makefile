##
## This file is part of the coreboot project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2008 Uwe Hermann <uwe@hermann-uwe.de>
## Copyright (C) 2009-2010 coresystems GmbH
## Copyright (C) 2011 secunet Security Networks AG
## Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 3. The name of the author may not be used to endorse or promote products
##    derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##


$(eval $(shell bash util/xcompile/xcompile $(XGCCPATH) > .xcompile))
include .xcompile

export top := $(CURDIR)
export src := src
export srck := $(top)/util/kconfig
export obj ?= build
export objutil ?= $(obj)/util
export objk := $(objutil)/kconfig


export KCONFIG_AUTOHEADER := $(obj)/config.h
export KCONFIG_AUTOCONFIG := $(obj)/auto.conf
export KCONFIG_DEPENDENCIES := $(obj)/auto.conf.cmd
export KCONFIG_SPLITCONFIG := $(obj)/config
export KCONFIG_TRISTATE := $(obj)/tristate.conf
export KCONFIG_NEGATIVES := 1

# directory containing the toplevel Makefile.inc
TOPLEVEL := .

CONFIG_SHELL := sh
KBUILD_DEFCONFIG := configs/defconfig
UNAME_RELEASE := $(shell uname -r)
DOTCONFIG ?= .config

###############################################################################
#
# DOTCONFIG= is an optional make command line parameter which can be used to
# specify a "full" format config file that will be used instead of .config.
# This makefile will run "oldconfig" to generate the config.h file if
# necessary. (Usage: make DOTCONFIG=configs/config.example)
#
# DEFCONFIG= is an optional make command line parameter which can be used to
# specify a "mini" format config file that will be used instead of .config.
# This makefile will automatically run "olddefconfig" to convert the "mini"
# config file into a full config file before continuing with the normal buld
# operation. (Usage: make DEFCONFIG=configs/config.example)
#
# NOTE: DEFCONFIG takes precedence over DOTCONFIG, if both are defined.
#
###############################################################################
ifeq ($(DEFCONFIG),)
KCONFIG_CONFIG = $(DOTCONFIG)
export KCONFIG_CONFIG
HAVE_DOTCONFIG := $(wildcard $(DOTCONFIG))
else
# Make sure that Kconfig uses the defconfig file provided when running
# olddefconfig below.
KCONFIG_CONFIG = $(DEFCONFIG)
export KCONFIG_CONFIG
HAVE_DOTCONFIG := $(wildcard $(DEFCONFIG))
endif

MAKEFLAGS += -rR --no-print-directory

# Make is silent per default, but 'make V=1' will show all compiler calls.
Q:=@
ifneq ($(V),1)
ifneq ($(Q),)
.SILENT:
endif
endif

HOSTCC = gcc
HOSTCXX = g++

# Add -MMD flag to all host util builds.  This generates dependency files
# that are utilized by the OSP script to deterimine which sources were
# actually built by SageBIOS for a given configuration.
HOSTCFLAGS := -g -MMD
HOSTCXXFLAGS := -g -MMD

DOXYGEN := doxygen
DOXYGEN_OUTPUT_DIR := doxygen

all: debug real-all

# Help text used by make help.  Needs to come before include util/kconfig/Makefile
# to appear in the right order
help::
	@echo  '*** Top level SageBIOS BSP targets ***'
	@echo  '  all        - builds coreboot based on your current .config'
	@echo  '  clean      - removes most of the build output - leaves payload dirs alone'
	@echo  '  distclean  - removes all of the build output and the .config'
	@echo  '  lint       - runs a set of checkers on coreboot source'
	@echo  '  doxygen    - Create or update coreboot doxygen documentation'
	@echo  '  osp        - Creates an Open Source Package'
	@echo  '  help       - Displays a list of targets'
	@echo
	@echo  '*** Debug targets ***'
	@echo  '  printall        - prints out coreboot variables and tool versions'
	@echo  '  debuginfo       - logs the coreboot variables and a build to debuginfo.log'
	@echo  '  ridiculousinfo  - logs coreboot variables and a build with all compiler calls'
	@echo
	@echo  '*** Kconfig targets ***'

# This include must come _before_ the pattern rules below!
# Order _does_ matter for pattern rules.
include util/kconfig/Makefile
include payloads/Makefile.inc

# Three cases where we don't need fully populated $(obj) lists:
# 1. when no .config exists
# 2. when make config (in any flavour) is run
# 3. when make distclean is run
# Don't waste time on reading all Makefile.incs in these cases
ifeq ($(strip $(HAVE_DOTCONFIG)),)
NOCOMPILE:=1
endif
ifneq ($(MAKECMDGOALS),)
ifneq ($(filter %config %clean cross%,$(MAKECMDGOALS)),)
NOCOMPILE:=1
endif
ifeq ($(MAKECMDGOALS), %clean)
NOMKDIR:=1
endif
endif

# Special classes are managed types with special behaviour
# On parse time, for each entry in variable $(1)-y
# a handler $(1)-handler is executed with the arguments:
# * $(1): directory the parser is in
# * $(2): current entry
add-special-class= \
	$(eval $(1):=) \
	$(eval special-classes+=$(1))

ifeq ($(NOCOMPILE),1)
include $(TOPLEVEL)/Makefile.inc
real-all: config

else

###############################################################################
#
# DEFCONFIG: Convert the "mini" config file format to a full config before
#            building.
#
###############################################################################
ifneq ($(DEFCONFIG),)
$(eval $(shell $(MAKE) DOTCONFIG=$(HAVE_DOTCONFIG) olddefconfig))
endif

include $(HAVE_DOTCONFIG)

ifeq ($(CONFIG_COMPILER_LLVM_CLANG),y)
# FIXME: armv7/aarch64 won't build right now
CFLAGS_x86_32 = -no-integrated-as -Qunused-arguments -target i386-elf -m32
CC_x86_32:=clang

ifneq ($(CONFIG_MMX),y)
CFLAGS_x86_32 += -mno-mmx
endif

# FIXME: we end up with conflicting flags with this, not clear on this part.
#ifneq ($(CONFIG_SSE),y)
#CFLAGS_x86_32 += -mno-sse
#endif

CFLAGS_armv7 = -no-integrated-as -Qunused-arguments -target armv7-eabi -ccc-gcc-name $(CC_armv7)
CC_armv7:=clang

CFLAGS_aarch64 = -no-integrated-as -Qunused-arguments -target aarch64-eabi -ccc-gcc-name $(CC_aarch64)
CC_aarch64:=clang

HOSTCC:=clang
endif

include toolchain.inc

strip_quotes = $(subst ",,$(subst \",,$(1)))

# The primary target needs to be here before we include the
# other files

real-all: real-target

# must come rather early
.SECONDEXPANSION:

$(obj)/config.h:
	$(MAKE) oldconfig

# Add a new class of source/object files to the build system
add-class= \
	$(eval $(1)-srcs:=) \
	$(eval $(1)-objs:=) \
	$(eval classes+=$(1))

# Clean -y variables, include Makefile.inc
# Add paths to files in X-y to X-srcs
# Add subdirs-y to subdirs
includemakefiles= \
	$(foreach class,classes subdirs $(classes) $(special-classes), $(eval $(class)-y:=)) \
	$(eval -include $(1)) \
	$(foreach class,$(classes-y), $(call add-class,$(class))) \
	$(foreach class,$(classes), \
		$(eval $(class)-srcs+= \
			$$(subst $(top)/,, \
			$$(abspath $$(subst $(dir $(1))/,/,$$(addprefix $(dir $(1)),$$($(class)-y))))))) \
	$(foreach special,$(special-classes), \
		$(foreach item,$($(special)-y), $(call $(special)-handler,$(dir $(1)),$(item)))) \
	$(eval subdirs+=$$(subst $(CURDIR)/,,$$(abspath $$(addprefix $(dir $(1)),$$(subdirs-y)))))

# For each path in $(subdirs) call includemakefiles
# Repeat until subdirs is empty
evaluate_subdirs= \
	$(eval cursubdirs:=$(subdirs)) \
	$(eval subdirs:=) \
	$(foreach dir,$(cursubdirs), \
		$(eval $(call includemakefiles,$(dir)/Makefile.inc))) \
	$(if $(subdirs),$(eval $(call evaluate_subdirs)))

# collect all object files eligible for building
subdirs:=$(TOPLEVEL)
$(eval $(call evaluate_subdirs))
ifeq ($(FAILBUILD),1)
$(error cannot continue build)
endif

# Eliminate duplicate mentions of source files in a class
$(foreach class,$(classes),$(eval $(class)-srcs:=$(sort $($(class)-srcs))))

src-to-obj=$(addsuffix .$(1).o, $(basename $(patsubst src/%, $(obj)/%, $($(1)-srcs))))
$(foreach class,$(classes),$(eval $(class)-objs:=$(call src-to-obj,$(class))))

# Save all objs before processing them (for dependency inclusion)
originalobjs:=$(foreach var, $(addsuffix -objs,$(classes)), $($(var)))

# Call post-processors if they're defined
$(foreach class,$(classes),\
	$(if $(value $(class)-postprocess),$(eval $(call $(class)-postprocess,$($(class)-objs)))))

allsrcs:=$(foreach var, $(addsuffix -srcs,$(classes)), $($(var)))
allobjs:=$(foreach var, $(addsuffix -objs,$(classes)), $($(var)))
alldirs:=$(sort $(abspath $(dir $(allobjs))))

# macro to define template macros that are used by use_template macro
define create_cc_template
# $1 obj class
# $2 source suffix (c, S)
# $3 additional compiler flags
# $4 additional dependencies
ifn$(EMPTY)def $(1)-objs_$(2)_template
de$(EMPTY)fine $(1)-objs_$(2)_template
$(obj)/$$(1).$(1).o: src/$$(1).$(2) $(obj)/config.h $(4)
	@printf "    CC         $$$$(subst $$$$(obj)/,,$$$$(@))\n"
	$(CC_$(1)) $(3) -MMD $$$$(CPPFLAGS_$(1)) $$$$(CFLAGS_$(1)) -c -o $$$$@ $$$$<
en$(EMPTY)def
end$(EMPTY)if
endef

filetypes-of-class=$(subst .,,$(sort $(suffix $($(1)-srcs))))
$(foreach class,$(classes), \
	$(foreach type,$(call filetypes-of-class,$(class)), \
		$(eval $(call create_cc_template,$(class),$(type),$($(class)-$(type)-ccopts),$($(class)-$(type)-deps)))))

foreach-src=$(foreach file,$($(1)-srcs),$(eval $(call $(1)-objs_$(subst .,,$(suffix $(file)))_template,$(subst src/,,$(basename $(file))))))
$(eval $(foreach class,$(classes),$(call foreach-src,$(class))))

DEPENDENCIES = $(originalobjs:.o=.d)
-include $(DEPENDENCIES)

printall:
	@echo '***** Objects *****'
	@$(foreach class,$(classes),echo "$(class)-objs:=$($(class)-objs)\
"; )
	@echo; echo '***** Directories *****'
	@echo alldirs:=$(alldirs)
	@echo; echo '***** Sources *****'
	@echo allsrcs=$(allsrcs)
	@echo; echo '***** Depencencies *****'
	@echo DEPENDENCIES=$(DEPENDENCIES)
	@echo; echo '***** Includes *****'
	@echo CPPFLAGS_common=$(CPPFLAGS_common)
	@echo CPPFLAGS_bootblock=$(CPPFLAGS_bootblock)
	@echo CPPFLAGS_romstage=$(CPPFLAGS_romstage)
	@echo CPPFLAGS_ramstage=$(CPPFLAGS_ramstage)
	@echo;echo '***** Libgcc Filename *****'
	@echo LIBGCC_FILE_NAME=$(LIBGCC_FILE_NAME_$(class))
	@echo;echo '***** Classes *****'
	@$(foreach class,$(special-classes),echo "$(class):='$($(class))'\
"; )
	@echo;echo '***** .config *****'
	@cat $(DOTCONFIG)
	@echo;echo '***** .xcompile *****'
	@cat .xcompile
	@echo;echo '***** environment *****'
	@set
	@echo; echo '***** current working directory *****'
	@pwd
	@echo;echo; echo '***** Tool Versions *****'
	@echo;echo '** shell: /bin/sh **'
	/bin/sh --version
	@echo;echo "** crosscompile CC: $(CC) **"
	$(CC) --version
	@echo;echo "** crosscompile CPP: $(CPP) **"
	$(CPP) --version
	@echo;echo "** crosscompile AS: $(AS) **"
	$(AS) --version
	@echo;echo "** crosscompile LD: $(LD) **"
	$(LD) --version
	@echo;echo "** crosscompile NM: $(NM) **"
	$(NM) --version
	@echo;echo "** crosscompile OBJCOPY: $(OBJCOPY) **"
	$(OBJCOPY) --version
	@echo;echo "** crosscompile OBJDUMP: $(OBJDUMP) **"
	$(OBJDUMP) --version
	@echo;echo "** crosscompile READELF: $(READELF) **"
	$(READELF) --version
	@echo;echo "** crosscompile STRIP: $(STRIP) **"
	$(STRIP) --version
	@echo;echo "** crosscompile AR: $(AR) **"
	$(AR) --version
	@echo;echo "** Host Compiler HOSTCC: $(HOSTCC) **"
	$(HOSTCC) --version
	@echo;echo "** Host Compiler HOSTCXX: $(HOSTCXX) **"
	$(HOSTCXX) --version
	@echo;echo "** Host awk **"
	which awk
	awk --version
ifneq ($(CCACHE),)
	@echo;echo "** Host CCACHE: $(CCACHE) **"
	$(CCACHE) --version
endif
	@echo;echo "** Host MAKE: $(MAKE) **"
	$(MAKE) --version
	@echo;echo "** Host sed **"
	sed --version
	@echo;echo "** Host python **"
	python --version 2>&1
	@echo;echo "** IASL **"
	which $(IASL)
	$(IASL) | grep -G 'version\|Specification Revision\|Intel'

endif

ifndef NOMKDIR
$(shell mkdir -p $(KCONFIG_SPLITCONFIG) $(objutil)/kconfig/lxdialog $(additional-dirs) $(alldirs))
endif

cscope:
	cscope -bR

doxy: doxygen
doxygen:
	$(DOXYGEN) documentation/Doxyfile.coreboot

doxyclean: doxygen-clean
doxygen-clean:
	rm -rf $(DOXYGEN_OUTPUT_DIR)

cleanall: clean clean-payloads

distcleanall: distclean distclean-payloads

clean-for-update: doxygen-clean clean-for-update-target
	rm -rf $(obj) .xcompile

clean: clean-for-update clean-target
	rm -f .ccwrap

clean-cscope:
	rm -f cscope.out

distclean: clean
	rm -f .config .config.old ..config.tmp .kconfig.d .tmpconfig* .ccwrap .xcompile debuginfo.log
	rm -rf coreboot-builds $(DEPS)

debuginfo:
	-@echo 'Saving build and system information to debuginfo.log'
	-@$(MAKE) cleanall > /dev/null
	-@$(MAKE) printall > debuginfo.log
	-@echo '' >> debuginfo.log
	-@echo '***** Oldconfig *****' >> debuginfo.log
	-@$(MAKE) oldconfig &>> debuginfo.log
	-@echo '' >> debuginfo.log
	-@echo '*************** Build ***************' >> debuginfo.log
	-@$(MAKE) &>> debuginfo.log

ridiculousinfo: debuginfo
	-@$(MAKE) cleanall > /dev/null
	-@echo '' >> debuginfo.log
	-@echo '*************** Debug Build ***************' >> debuginfo.log
	-@$(MAKE) --warn-undefined-functions -n &>> debuginfo.log
	-@$(MAKE) clean > /dev/null
	-@echo '' >> debuginfo.log
	-@echo '*************** Debug Make Build ***************' >> debuginfo.log
	-@$(MAKE) -p -d &>> debuginfo.log

objdump: real-all
ifeq ($(CONFIG_ARCH_X86),y)
	objdump -M i386 -S $(objcbfs)/ramstage.elf  > $(objcbfs)/ramstage.dump
	objdump -M i386 -S $(objcbfs)/romstage_xip.elf  > $(objcbfs)/romstage.dump
	$(eval offset:=$(shell $(obj)/cbfstool $(obj)/coreboot.rom print | grep -oP "bootblocksize\s+\K\d+"))
	$(eval offset:=$(shell echo "$$((0xffffffff - $(offset) + 1))" ))
	objdump -M i386 -b binary --disassemble-all --architecture=i386 --adjust-vma=$(offset) $(objcbfs)/bootblock.bin > $(objcbfs)/bootblock.dump
	echo "Objdump files generated in $(objcbfs)"
else
	echo "The objdump target currently only works for x86 platforms"
endif

# To use the debug target, create a new target 'debug:' in any of the makefiles.
# This target will be run before any other targets, allowing variables to be
# printed.  You can use the 'pause' or 'sleep' commands in that target to delay
# the build.
debug::

# Make the "Open Source Package" based on the currently configured build.  This
# will generate a source file package, to meet open source publishing
# requirements, that includes only the source files necessary to generate the
# shipping binary file.  The open source package is not configurable, the build
# process is managed by a shell script instead of Makefiles, and the final
# OSP will only include source files for files in CBFS (coreboot file system)
# that have an open source publishing requirement.
osp: all
	../util/package_osp.pl --checkpkg


.PHONY: $(PHONY) osp debug clean clean-for-update clean-cscope cscope distclean doxygen doxy .xcompile debuginfo
