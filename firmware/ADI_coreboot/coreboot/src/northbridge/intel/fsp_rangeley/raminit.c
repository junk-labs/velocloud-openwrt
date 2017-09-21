/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Google Inc.
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <console/console.h>
#include <string.h>
#include <arch/io.h>
#include <cbmem.h>
#include <device/pci_def.h>
#include "raminit.h"
#include "northbridge.h"
#include <drivers/intel/fsp/fsp_util.h>

/* TODO. We will need this once we decode Sideband registers.
 static const char* ecc_decoder[] = {
	"inactive",
	"active on IO",
	"disabled on IO",
	"active"
};
*/

/*
 * Dump in the log memory controller configuration as read from the memory
 * controller registers.
 */
void report_memory_config(void)
{
	/* TODO Sideband Register DRP setting */
	printk(BIOS_DEBUG, "TODO Sideband Register DRP setting \n");
}

unsigned long get_top_of_ram(void)
{
	/*
	 * Get top of usable (low) DRAM.
	 * FSP puts SMM just below top of low physical (BMBOUND)
	 */
	u32 tom = sideband_read(B_UNIT, BMBOUND);
	u32 bsmmrrl = sideband_read(B_UNIT, BSMMRRL) << 20;
	if (bsmmrrl) {
		tom = bsmmrrl;
	}
	tom -= FSP_RESERVE_MEMORY_SIZE;

	return (unsigned long) tom;
}
