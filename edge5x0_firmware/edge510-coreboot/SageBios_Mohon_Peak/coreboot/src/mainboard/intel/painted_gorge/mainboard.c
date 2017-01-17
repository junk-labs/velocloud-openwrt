/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <types.h>
#include <string.h>
#include <device/device.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <device/pci_ops.h>
#include <console/console.h>
#if CONFIG_PCI_ROM_RUN || CONFIG_VGA_ROM_RUN
#include <x86emu/x86emu.h>
#endif
#include <pc80/mc146818rtc.h>
#include <arch/acpi.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <boot/coreboot_tables.h>
#include <southbridge/intel/fsp_rangeley/soc.h>


/*
 * mainboard_enable is executed as first thing after enumerate_buses().
 * This is the earliest point to add customization.
 */
static void mainboard_enable(device_t dev)
{
/* Remove when painted gorge gets an RTC battery */
#define MIN_YEAR		0x14
#define MAX_YEAR		0x34
#define CURRENT_CENTURY	0x20
	if ( (cmos_read(RTC_CLK_YEAR) > MAX_YEAR) ||
		 (cmos_read(RTC_CLK_YEAR) < MIN_YEAR) ||
		 (cmos_read(RTC_CLK_ALTCENTURY) > CURRENT_CENTURY) ||
		 (cmos_read(RTC_CLK_ALTCENTURY) < CURRENT_CENTURY))
			rtc_update_cmos_date(RTC_HAS_ALTCENTURY);
	}

struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};

