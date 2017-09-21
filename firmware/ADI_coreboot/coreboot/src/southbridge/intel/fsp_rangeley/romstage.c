/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <timestamp.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <device/pnp_def.h>
#include <cpu/x86/lapic.h>
#include <pc80/mc146818rtc.h>
#include <cbmem.h>
#include <console/console.h>
#include <console/usb.h>
#include <drivers/intel/fsp/fsp_util.h>
#include "northbridge/intel/fsp_rangeley/northbridge.h"
#include "northbridge/intel/fsp_rangeley/raminit.h"
#include "southbridge/intel/fsp_rangeley/soc.h"
#include "southbridge/intel/fsp_rangeley/gpio.h"
#include "southbridge/intel/fsp_rangeley/romstage.h"
#include <arch/cpu.h>
#include <arch/stages.h>
#include <cpu/x86/msr.h>
#include "gpio.h"

void main(FSP_INFO_HEADER *fsp_info_header)
{
	uint32_t pm1_cnt;
	uint16_t pm1_sts;
	uint32_t fd_mask = 0;
	uint32_t func_dis = DEFAULT_PBASE + PBASE_FUNC_DIS;
	/*
	 * Do not use the Serial Console before it is setup.
	 * This causes the I/O to clog and a side effect is
	 * that the reset button stops functioning.  So
	 * instead just use outb so it doesn't output to the
	 * console when CONFIG_CONSOLE_POST.
	 */
	outb(0x40, 0x80);

#if CONFIG_COLLECT_TIMESTAMPS
	save_timestamp_to_cmos(CMOS_MAIN_START_ADDR, rdtsc());
#endif

	/*
	 * Rangeley UART POR state is enabled
	 * Rangeley uses UART1 (COM2, 0x2F8)
	 */
	console_init();
	post_code(0x41);

	/* Enable GPIOs BAR */
	pci_write_config32(SOC_LPC_DEV, GBASE, DEFAULT_GPIOBASE|0x02);

	early_mainboard_romstage_entry();

	post_code(0x42);
	rangeley_sb_early_initialization();

	post_code(0x43);
	rangeley_early_initialization();
	printk(BIOS_DEBUG, "Back from rangeley_early_initialization()\n");

	post_code(0x44);
	/* Check PM1_STS[15] to see if we are waking from Sx */
	pm1_sts = inw(DEFAULT_ABASE + PM1_STS);

	/* Read PM1_CNT[12:10] to determine which Sx state */
	pm1_cnt = inl(DEFAULT_ABASE + PM1_CNT);
	post_code(0x45);
	if ((pm1_sts & WAK_STS) && ((pm1_cnt >> 10) & 7) == 5) {
		printk(BIOS_DEBUG, "Resume from S3 detected, but disabled.\n");
	}

	post_code(0x46);

	/* Program any required function disables */
	get_func_disables(&fd_mask);

	if (fd_mask != 0) {
		write32(func_dis, read32(func_dis) | fd_mask);
		/* Ensure posted write hits. */
		read32(func_dis);
	}

#if CONFIG_COLLECT_TIMESTAMPS
	save_timestamp_to_cmos(CMOS_PRE_INITRAM_ADDR, rdtsc());
#endif

  /*
   * Call early init to initialize memory and chipset. This function returns
   * to the romstage_main_continue function with a pointer to the HOB
   * structure.
   */
	post_code(0x47);
	printk(BIOS_DEBUG, "Starting the Intel FSP (early_init)\n");
	fsp_early_init(fsp_info_header);
	die("Uh Oh! fsp_early_init should not return here.\n");
}

/*
 * The FSP early_init function returns to this function.
 * Memory is setup and the stack is set by the FSP.
 */
void romstage_main_continue(EFI_STATUS status, void *hob_list_ptr) {
	int cbmem_was_initted;
	void *cbmem_hob_ptr;

#if CONFIG_COLLECT_TIMESTAMPS
	tsc_t after_initram_time = rdtsc();
	tsc_t base_time;
	tsc_t before_car_time;
	tsc_t after_car_time;
	tsc_t start_romstage_time;
	tsc_t before_initram_time;

	get_timestamp_from_cmos(CMOS_BASETIME_READ_ADDR, &base_time);
	get_timestamp_from_cmos(CMOS_PRE_CAR_READ_ADDR, &before_car_time);
	get_timestamp_from_cmos(CMOS_POST_CAR_READ_ADDR, &after_car_time);
	get_timestamp_from_cmos(CMOS_MAIN_START_ADDR, &start_romstage_time);
	get_timestamp_from_cmos(CMOS_PRE_INITRAM_ADDR, &before_initram_time);

/* Unless there's a reason to save the TSC offset, ignore it */
if (base_time.hi == 0)
	base_time.lo = 0;
#endif
	tco_watchdog_init();

	post_code(0x48);
	printk(BIOS_DEBUG, "%s status: %x  hob_list_ptr: %x\n",
		__func__, (u32) status, (u32) hob_list_ptr);

#if IS_ENABLED(CONFIG_USBDEBUG_IN_ROMSTAGE)
	/* FSP reconfigures USB, so reinit it to have debug */
	usbdebug_init();
#endif	/* IS_ENABLED(CONFIG_USBDEBUG_IN_ROMSTAGE) */

	/* For reference print FSP version */
	/*ToDo, where is this sideband?
	 * u32 version = MCHBAR32(0x5034);
	printk(BIOS_DEBUG, "FSP Version %d.%d.%d Build %d\n",
		version >> 24 , (version >> 16) & 0xff,
		(version >> 8) & 0xff, version & 0xff);
*/
	printk(BIOS_DEBUG, "FSP Status: 0x%0x\n", (u32)status);

	report_platform_info();
	report_memory_config();

	post_code(0x4b);
	late_mainboard_romstage_entry();

	post_code(0x4c);

	/* Decode E0000 and F0000 segment to DRAM */
	/* TODO: Move to raminit with FSP early_int call */
	sideband_write(B_UNIT, BMISC, sideband_read(B_UNIT, BMISC) | (1 << 1) | (1 << 0));

	quick_ram_check();
	post_code(0x4d);

	cbmem_was_initted = !cbmem_recovery(0);

	/* Save the HOB pointer in CBMEM to be used in ramstage*/
	cbmem_hob_ptr = cbmem_add (CBMEM_ID_HOB_POINTER, sizeof(*hob_list_ptr));
	*(u32*)cbmem_hob_ptr = (u32)hob_list_ptr;
	post_code(0x4e);

#if CONFIG_COLLECT_TIMESTAMPS
	timestamp_init(base_time);
	timestamp_reinit();
	timestamp_add(TS_BEFORE_CAR_INIT, before_car_time );
	timestamp_add(TS_AFTER_CAR_INIT, after_car_time );
	timestamp_add(TS_START_ROMSTAGE, start_romstage_time );
	timestamp_add(TS_BEFORE_INITRAM, before_initram_time );
	timestamp_add(TS_AFTER_INITRAM, after_initram_time);
	timestamp_add_now(TS_END_ROMSTAGE);
#endif
#if CONFIG_CONSOLE_CBMEM
	/* Keep this the last thing this function does. */
	cbmemc_reinit();
#endif

	/* Load the ramstage. */
	copy_and_run();
	while (1);
}
