###############################################################################
#
# Makefile - Wrapper for ipxe/src/Makefile, added for consistency between
#            payloads on Jenkins
#
# Copyright (c) 2013 Sage Electronic Engineering.  All rights reserved.
# Software License Agreement
#
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
# Sage Electronic Engineering SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR
# SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
#
##############################################################################

# Configure build with Sage toolchain
$(eval $(shell bash ../xcompile.sh > .xcompile))
include .xcompile

# Set CROSS_COMPILE prefix which is used by the ipxe/src/Makefile
export CROSS_COMPILE = $(GCCPREFIX)

# The Version string can be supplied by the environment:
# export VERSION = "Test_Version"

# The Target ROM name can be supplied by the environment:
# export PCI_VENDOR_DEVICE=10ec8168

ifeq ($(PCI_VENDOR_DEVICE),)
# Default behavior if PCI_VENDOR_DEVICE is not defined in the environment
all everything vmware:
	$(MAKE) -C src $@
else
# Example: export PCI_VENDOR_DEVICE=10ec8168; make
#          is equivalent to "make bin/10ec8168.rom"
# This can also build a list of PCI_VENDOR_DEVICE values
#          export PCI_VENDOR_DEVICE="80861502 80860438"; make
#          is equivalent to "make bin/80861502.rom bin/80860438.rom"
# The nested make function calls below do the following to PCI_VENDOR_DEVICE
#          1. Remove "" from around the list of PCI IDs
#          2. Add the suffix => .rom to each PCI ID in the list
#          3. Add the prefix => bin/ to each PCI Id in the list
all everything vmware:
	@echo "Starting ipxe build in src"
	$(MAKE) -C src $(addprefix bin/,$(addsuffix .rom,$(subst ",,$(PCI_VENDOR_DEVICE))))
	@echo "Finished ipxe build in src"
endif

# This is an implicit rule that is invoked for all "make bin/10ec8168.rom"
# commands.  It does the same as the explicit rule above, but you cannot
# mix implicit and explicit rules together.
bin/%.rom bin/%.lkrn:
	@echo "Starting ipxe build in src"
	$(MAKE) -C src $@
	@echo "Finished ipxe build in src"

clean veryclean:
	rm .xcompile
	$(MAKE) -C src $@

# Redirect "make distclean" to "make veryclean", this gives ipxe the same
# makefile interface that the other coreboot payloads have.
distclean:
	rm .xcompile
	$(MAKE) -C src veryclean

.PHONY: all everything vmware clean veryclean distclean
