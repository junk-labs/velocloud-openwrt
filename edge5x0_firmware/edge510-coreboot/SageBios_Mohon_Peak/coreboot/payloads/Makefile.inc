##
## This file is part of the coreboot project.
##
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

###############################################################################
# This makefile gets included into the Makefile sequence twice.  The first time,
# we just grab the clean targets.  The second time it's included we pull in 
# everything else.
#
# This is due to how and where it gets included.  The coreboot makefile expects
# to pull things in based on the .configs and doesn't pull in all of the
# makefiles every time.  This causes the clean targets to not get pulled in when
# we're doing a clean.
ifeq ($(CLEAN_PAYLOADS_INCLUDED),)
###############################################################################
# clean
###############################################################################
CLEAN_PAYLOADS_INCLUDED := 1
payload_dir = payloads
payload_list := $(filter-out payloads/Kconfig payloads/Makefile.inc \
	payloads/xcompile.sh, $(wildcard $(payload_dir)/*))
customer_payload_dir = $(payload_dir)/customer
customer_payload_dirs = $(wildcard $(customer_payload_dir)/*)
customer_payloads_list = $(foreach cust_directory, $(customer_payload_dirs), \
	$(wildcard $(cust_directory)/*))
payload_list += $(customer_payloads_list)

clean-payloads:
	-$(if $(strip $(payload_list)), \
	$(foreach payload, $(payload_list), \
	[ -f $(payload)/Makefile ] && echo "cleaning $(payload)" && \
	$(MAKE) -w -C $(payload) clean ; ), \
	echo "No valid payloads to clean." )

distclean-payloads:
	-$(if $(strip $(payload_list)), \
	$(foreach payload, $(payload_list), \
	[ -f $(payload)/Makefile ] && echo "distcleaning $(payload)" && \
	$(MAKE) -w -C $(payload) distclean ; ), \
	echo "No valid payloads to clean." )

help::
	@echo
	@echo  '*** Payload targets ***'
	@echo  '  clean-payloads      - cleans all of the payload directories'
	@echo  '  distclean-payloads  - runs distclean on all of the payload directories'

else # ifeq ($(CLEAN_PAYLOADS_INCLUDED),)

###############################################################################
# Memtest
###############################################################################
ifeq ($(CONFIG_PAYLOAD_MEMTEST),y)

MEMTEST_DIR := payloads/memtest86+
ifeq ($(wildcard $(MEMTEST_DIR)),)
$(error Error: Memtest source directory does not exist)
endif # ($(wildcard $(MEMTEST_DIR)),)

MEMTEST_BINARY := $(MEMTEST_DIR)/memtest.elf
COREBOOT_ROM_COMPONENTS += $(MEMTEST_BINARY)

ifeq ($(CONFIG_MEMTEST_COMPRESSED_LZMA),y)
MEMTEST_COMPRESS_FLAG := -c LZMA
endif

ifneq ($(shell printf "%d" $(CONFIG_MEMTEST_PAYLOAD_BASE)),0)
MEMTEST_LOCATION := -b $(CONFIG_MEMTEST_PAYLOAD_BASE)
endif

ifeq ($(CONFIG_MEMTEST_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y0)
MEMTEST_TARGET := com1
else ifeq ($(CONFIG_MEMTEST_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y1)
MEMTEST_TARGET := com2
else ifeq ($(CONFIG_MEMTEST_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y2)
MEMTEST_TARGET := com3
else ifeq ($(CONFIG_MEMTEST_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y3)
MEMTEST_TARGET := com4
else ifeq ($(CONFIG_MEMTEST_SERIAL_COM1),y)
MEMTEST_TARGET := com1
else ifeq ($(CONFIG_MEMTEST_SERIAL_COM2),y)
MEMTEST_TARGET := com2
else ifeq ($(CONFIG_MEMTEST_SERIAL_COM3),y)
MEMTEST_TARGET := com3
else ifeq ($(CONFIG_MEMTEST_SERIAL_COM4),y)
MEMTEST_TARGET := com4
else
MEMTEST_TARGET:=all
endif

ifeq ($(CONFIG_MEMTEST_CBFS_NAME), "")
$(error Error: The name for the Memtest rom in cbfs is blank)
endif

# By default MAKEFLAGS disables all implicit rules and variables.
# The memtest makefile relies on the implicit rules, so override the MAKEFLAGS
# variable for the memtest build.
$(MEMTEST_BINARY): MAKEFLAGS :=

$(MEMTEST_BINARY): $(obj)/coreboot.pre $(obj)/build.h
	@echo "Starting Memtest86+ payload build in $(MEMTEST_DIR)"
	@echo $(MAKE) -C $(MEMTEST_DIR) $(MEMTEST_TARGET)
	$(MAKE) -C $(MEMTEST_DIR) $(MEMTEST_TARGET)
	@echo "Finished Memtest86+ payload build in $(MEMTEST_DIR)"
	@printf "    PAYLOAD    $(obj)/coreboot.pre add-payload -f $(MEMTEST_BINARY) -n $(CONFIG_MEMTEST_CBFS_NAME) $(MEMTEST_LOCATION) $(MEMTEST_COMPRESS_FLAG)\n"
	$(CBFSTOOL) $(obj)/coreboot.pre add-payload -f $(MEMTEST_BINARY) -n $(CONFIG_MEMTEST_CBFS_NAME) $(MEMTEST_LOCATION) $(MEMTEST_COMPRESS_FLAG)

endif # ifeq ($(CONFIG_PAYLOAD_MEMTEST),y)

###############################################################################
# iPXE
###############################################################################
ifeq ($(CONFIG_PXE_ROM),y)

# Always use the top-level makefile which is responsible for configuring the
# toolchain for the ipxe/src/Makefile.  PXE will not build successfully in
# cygwin if ipxe/src/Makefile is called from here.
IPXE_DIR := payloads/ipxe
ifeq ($(wildcard $(IPXE_DIR)),)
$(error Error: iPXE source directory does not exist)
endif # ($(wildcard $(IPXE_DIR)),)

ifeq ($(CONFIG_PXE_ROM_ID), "")
$(error Error: The Vendor ID device ID for the pxe rom is blank.)
endif
comma := ,
pxe_rom_filename := bin/$(call strip_quotes,$(subst $(comma),,$(CONFIG_PXE_ROM_ID))).rom

COREBOOT_ROM_DEPENDENCIES+=$(IPXE_DIR)/src/$(pxe_rom_filename)


ifeq ($(CONFIG_PXE_ROM_PUT_IN_GENROMS),y)
pxe_rom_cbfs_name := genroms/pxe.rom
else
pxe_rom_cbfs_name := pci$(call strip_quotes,$(CONFIG_PXE_ROM_ID)).rom
endif

ifneq ($(shell printf "%d" $(CONFIG_PXE_ROM_ADDRESS)),0)
pxe_rom_position := -b $(CONFIG_PXE_ROM_ADDRESS)
endif

ifeq ($(CONFIG_PXE_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y0)
IPXE_SERIAL_PORT := COMCONSOLE=COM1
else ifeq ($(CONFIG_PXE_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y1)
IPXE_SERIAL_PORT := COMCONSOLE=COM2
else ifeq ($(CONFIG_PXE_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y2)
IPXE_SERIAL_PORT := COMCONSOLE=COM3
else ifeq ($(CONFIG_PXE_USE_CONSOLE_COM_PORT)$(CONFIG_UART_FOR_CONSOLE),y3)
IPXE_SERIAL_PORT := COMCONSOLE=COM4
else ifeq ($(CONFIG_PXE_SERIAL_COM1),y)
IPXE_SERIAL_PORT := COMCONSOLE=COM1
else ifeq ($(CONFIG_PXE_SERIAL_COM2),y)
IPXE_SERIAL_PORT :=cCOMCONSOLE=COM2
else ifeq ($(CONFIG_PXE_SERIAL_COM3),y)
IPXE_SERIAL_PORT := COMCONSOLE=COM3
else ifeq ($(CONFIG_PXE_SERIAL_COM4),y)
IPXE_SERIAL_PORT := COMCONSOLE=COM4
endif

## Make target to automatically build the PXE ROM if necessary.
## This target will generate the named PXE ROM file as specified in the config
## options, assuming that the file is generated from the ipxe source located
## in the payloads directory.
$(IPXE_DIR)/src/$(pxe_rom_filename): $(obj)/coreboot.pre $(obj)/build.h
	@echo "Starting iPXE payload build in $(IPXE_DIR)"
	@echo "$(MAKE) -C $(IPXE_DIR) $(pxe_rom_filename) $(IPXE_SERIAL_PORT)"
	$(MAKE) -C $(IPXE_DIR) $(pxe_rom_filename) $(IPXE_SERIAL_PORT)
	@echo "Finished iPXE payload build in $(IPXE_DIR)"
	@printf "    PAYLOAD    $(obj)/coreboot.pre add-payload -f $(pxe_rom_filename) -n $(pxe_rom_cbfs_name) $(pxe_rom_position) -t raw\n"
	$(CBFSTOOL) $(obj)/coreboot.pre add -f $(IPXE_DIR)/src/$(pxe_rom_filename) -n $(pxe_rom_cbfs_name) $(pxe_rom_position) -t raw

endif # ifeq ($(CONFIG_PXE_ROM),y)


###############################################################################
# SeaBIOS
###############################################################################

SEABIOS_DIR := payloads/seabios
ifneq ($(wildcard $(SEABIOS_DIR)),)# if the payloads/seabios dir is missing, skip this

$(SEABIOS_DIR)/%.elf:
	@echo "Starting SeaBIOS payload build in $(SEABIOS_DIR)"
	@printf "    PAYLOAD    $@\n"
	@echo "$(MAKE) -C $(SEABIOS_DIR) \
	  $(if $(call strip_quotes(CONFIG_PAYLOAD_USE_CONFIG_FILE)),, \
	  DOTCONFIG=$(addprefix configs/,$(notdir $(call strip_quotes, \
	  $(CONFIG_PAYLOAD_USE_CONFIG_FILE))))) olddefconfig"
	$(MAKE) -C $(SEABIOS_DIR) \
	  $(if $(call strip_quotes(CONFIG_PAYLOAD_USE_CONFIG_FILE)),, \
	  DOTCONFIG=$(addprefix configs/,$(notdir $(call strip_quotes, \
	  $(CONFIG_PAYLOAD_USE_CONFIG_FILE))))) olddefconfig
	@echo "$(MAKE) -C $(SEABIOS_DIR) \
	  $(if $(call strip_quotes(CONFIG_PAYLOAD_USE_CONFIG_FILE)),, \
	  DOTCONFIG=$(addprefix configs/,$(notdir $(call strip_quotes, \
	  $(CONFIG_PAYLOAD_USE_CONFIG_FILE))))) all"
	$(MAKE) -C $(SEABIOS_DIR) \
	  $(if $(call strip_quotes(CONFIG_PAYLOAD_USE_CONFIG_FILE)),, \
	  DOTCONFIG=$(addprefix configs/,$(notdir $(call strip_quotes, \
	  $(CONFIG_PAYLOAD_USE_CONFIG_FILE))))) all
	@echo "Finished SeaBIOS payload build in $(SEABIOS_DIR)"
endif

###############################################################################
# VGA BIOS
###############################################################################
ifeq ($(CONFIG_VGA_BIOS),y)
ifneq ($(findstring payloads,$(CONFIG_VGA_BIOS_FILE)),)
$(call strip_quotes,$(CONFIG_VGA_BIOS_FILE)):
	@echo "Starting VGA BIOS build in $(dir $@)"
	@printf "    VGA BIOS   $@\n"
	$(MAKE) -C $(dir $@)
	@echo "Finished VGA BIOS build in $(dir $@)"

endif
endif

###############################################################################
# Extra payloads
###############################################################################

ifeq ($(CONFIG_EXTRA_1ST_PAYLOAD),y)
EXTRA_1ST_PAYLOAD_FILE=$(call strip_quotes,$(CONFIG_EXTRA_1ST_PAYLOAD_FILE))
COREBOOT_ROM_DEPENDENCIES+=$(EXTRA_1ST_PAYLOAD_FILE)
endif
ifeq ($(CONFIG_EXTRA_2ND_PAYLOAD),y)
EXTRA_2ND_PAYLOAD_FILE:=$(call strip_quotes,$(CONFIG_EXTRA_2ND_PAYLOAD_FILE))
COREBOOT_ROM_DEPENDENCIES+=$(EXTRA_2ND_PAYLOAD_FILE)
endif
ifeq ($(CONFIG_EXTRA_3RD_PAYLOAD),y)
EXTRA_3RD_PAYLOAD_FILE=$(call strip_quotes,$(CONFIG_EXTRA_3RD_PAYLOAD_FILE))
COREBOOT_ROM_DEPENDENCIES+=$(EXTRA_3RD_PAYLOAD_FILE)
endif
ifeq ($(CONFIG_EXTRA_4TH_PAYLOAD),y)
EXTRA_4TH_PAYLOAD_FILE=$(call strip_quotes,$(CONFIG_EXTRA_4TH_PAYLOAD_FILE))
COREBOOT_ROM_DEPENDENCIES+=$(EXTRA_4TH_PAYLOAD_FILE)
endif

payloads/%.elf:
	@echo "Starting payload build in $(dir $@)"
	@printf "    PAYLOAD    $@\n"
	$(MAKE) -C $(dir $@)
	@echo "Finished payload build in $(dir $@)"

endif # ifeq ($(CLEAN_PAYLOADS_INCLUDED),)
