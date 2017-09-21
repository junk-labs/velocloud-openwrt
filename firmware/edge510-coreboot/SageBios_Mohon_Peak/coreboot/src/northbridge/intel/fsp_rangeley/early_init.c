/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
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

#include <stdint.h>
#include <stdlib.h>
#include <console/console.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <elog.h>
#include "northbridge.h"

static void rangeley_setup_bars(void)
{
	printk(BIOS_DEBUG, "Setting up static northbridge registers...");
	/* TODO Set up all hardcoded northbridge BARs */

#if CONFIG_ELOG_BOOT_COUNT
	/* Increment Boot Counter for non-S3 resume */
	if ((inw(DEFAULT_ABASE + PM1_STS) & WAK_STS) &&
	    ((inl(DEFAULT_ABASE + PM1_CNT) >> 10) & 7) != SLP_TYP_S3)
		boot_count_increment();
#endif

	printk(BIOS_DEBUG, " done.\n");

#if CONFIG_ELOG_BOOT_COUNT
	/* Increment Boot Counter except when resuming from S3 */
	if ((inw(DEFAULT_ABASE + PM1_STS) & WAK_STS) &&
	    ((inl(DEFAULT_ABASE + PM1_CNT) >> 10) & 7) == SLP_TYP_S3)
		return;
	boot_count_increment();
#endif
}

void rangeley_early_initialization(void)
{
	/* Setup all BARs required for early PCIe and raminit */
	rangeley_setup_bars();
}
