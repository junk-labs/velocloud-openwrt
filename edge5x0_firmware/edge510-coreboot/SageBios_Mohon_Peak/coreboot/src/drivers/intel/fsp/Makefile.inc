#
# This file is part of the coreboot project.
#
# Copyright (C) 2014 Sage Electronic Engineering, LLC.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#

ramstage-y += fsp_util.c
romstage-y += fsp_util.c
ramstage-$(CONFIG_ENABLE_MRC_CACHE) += fastboot_cache.c
romstage-$(CONFIG_ENABLE_MRC_CACHE) += fastboot_cache.c

CPPFLAGS_common += -Isrc/drivers/intel/fsp
CPPFLAGS_common += -I$(dir $(call strip_quotes,$(CONFIG_FSP_FILE)))include

ifeq ($(CONFIG_USE_GENERIC_FSP_CAR_INC),y)
cpu_incs += $(src)/drivers/intel/fsp/cache_as_ram.inc
endif

ifeq ($(CONFIG_FSP_VENDORCODE_HEADER_PATH),)
fsp_dir := $(dir $(call strip_quotes,$(CONFIG_FSP_FILE)))
fsp_output_dir := $(obj)/drivers/intel/fsp
vendorcode_fsp_dir := $(fsp_dir:../vendorcode/)
fsp_src_dir := $(fsp_dir)src
fsp_inc_dir := $(fsp_dir)inc

fsp_c_inputs := $(foreach file, $(fsp_src_files), $(fsp_src_dir)/$(notdir $(file)))
fsp_c_outputs := $(foreach file,$(fsp_src_files), $(top)/$(fsp_output_dir)/$(notdir $(file)))
fsp_c_outfiles := $(foreach file,$(fsp_src_files), $(fsp_output_dir)/$(notdir $(file)))

ramstage-y += $(fsp_c_outputs)
ifeq ($(add_fsp_files_to_romstage),y)
romstage-y += $(fsp_c_outputs)
endif

$(fsp_c_outfiles): $(fsp_c_inputs) $(obj)/config.h
	cp $(fsp_src_dir)/$(notdir $@) $@

ifeq ($(CONFIG_HAVE_FSP_BIN),y)
cbfs-files-y += fsp.bin
fsp.bin-file := $(call strip_quotes,$(CONFIG_FSP_FILE))
fsp.bin-position := $(CONFIG_FSP_LOC)
fsp.bin-type := 0xab
endif
endif

ifeq ($(CONFIG_ENABLE_MRC_CACHE),y)
$(obj)/mrc.cache:
	dd if=/dev/zero count=1 \
	bs=$(shell printf "%d" $(CONFIG_MRC_CACHE_SIZE) ) | \
	tr '\000' '\377' > $@

cbfs-files-y += mrc.cache
mrc.cache-file := $(obj)/mrc.cache
mrc.cache-position := $(CONFIG_MRC_CACHE_LOC)
mrc.cache-type := 0xac
endif
