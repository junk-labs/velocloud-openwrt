/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Google Inc.
 * Copyright (C) 2014 Sage Electronic Engineering, LLC.
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

#include <stdint.h>
#include <arch/cpu.h>
#include <cpu/x86/cache.h>
#include <cpu/x86/msr.h>
#include <cpu/x86/mtrr.h>
#include <arch/io.h>

#include "haswell.h"

#if CONFIG_SOUTHBRIDGE_INTEL_FSP_LYNXPOINT
/* Needed for RCBA access to set Soft Reset Data register */
#include <southbridge/intel/fsp_lynxpoint/pch.h>
#else
#error "CPU must be paired with Intel FSP_LynxPoint southbridge"
#endif


static void check_for_clean_reset(void)
{
	msr_t msr;
	msr = rdmsr(MTRRdefType_MSR);

	/* Use the MTRR default type MSR as a proxy for detecting INIT#.
	 * Reset the system if any known bits are set in that MSR. That is
	 * an indication of the CPU not being properly reset. */
	if (msr.lo & (MTRRdefTypeEn | MTRRdefTypeFixEn)) {
		outb(0x0, 0xcf9);
		outb(0x6, 0xcf9);
		while (1) {
			asm("hlt");
		}
	}
}

static void bootblock_cpu_init(void)
{
	check_for_clean_reset();
}
