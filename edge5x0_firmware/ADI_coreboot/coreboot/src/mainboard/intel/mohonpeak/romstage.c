/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
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
#include <drivers/intel/fsp/fsp_util.h>
#include <northbridge/intel/fsp_rangeley/northbridge.h>
#include <southbridge/intel/fsp_rangeley/soc.h>
#include <southbridge/intel/fsp_rangeley/gpio.h>
#include <southbridge/intel/fsp_rangeley/romstage.h>
#include <arch/cpu.h>
#include <cpu/x86/msr.h>
#include <reset.h>
#include "gpio.h"

static void interrupt_routing_config(void)
{
    u32 ilb_base = pci_read_config32(SOC_LPC_DEV, IBASE) & ~0xf;

    /*
     * Initialize Interrupt Routings for each device in ilb_base_address.
     * IR01 map to PCIe device 0x01 ... IR31 to device 0x1F.
     * PIRQ_A maps to IRQ 16 ... PIRQ_H maps tp IRQ 23.
     * This should match devicetree and the ACPI IRQ routing/
     */
	write32(ilb_base + ILB_ACTL, 0x0000);  /* ACTL bit 2:0 SCIS IRQ9 */
	write16(ilb_base + ILB_IR01, 0x3210);  /* IR01h IR(ABCD) - PIRQ(ABCD) */
	write16(ilb_base + ILB_IR02, 0x3210);  /* IR02h IR(ABCD) - PIRQ(ABCD) */
	write16(ilb_base + ILB_IR03, 0x7654);  /* IR03h IR(ABCD) - PIRQ(EFGH) */
	write16(ilb_base + ILB_IR04, 0x7654);  /* IR04h IR(ABCD) - PIRQ(EFGH) */
	write16(ilb_base + ILB_IR20, 0x7654);  /* IR14h IR(ABCD) - PIRQ(EFGH) */
	write16(ilb_base + ILB_IR22, 0x0007);  /* IR16h IR(A) - PIRQ(H) */
	write16(ilb_base + ILB_IR23, 0x0003);  /* IR17h IR(A) - PIRQ(D) */
	write16(ilb_base + ILB_IR24, 0x0003);  /* IR18h IR(A) - PIRQ(D) */
	write16(ilb_base + ILB_IR31, 0x0020);  /* IR1Fh IR(B) - PIRQ(C) */
}

/**
 * /brief mainboard call for setup that needs to be done before fsp init
 *
 */
void early_mainboard_romstage_entry(void)
{
	u8 val;

	struct i2c_gpio_dev bb_i2c1 = {
          .sdl_pin = 11,
          .scl_pin = 12,
	};

	console_init();

	vc_register_i2c_gpio(&bb_i2c1);

	/* Mode1 Select register  */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x0, 0x1);

	/* LED1 OUT (off) */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xd, 0x0);

	if ((cmos_read(0x10) & 1)) {

		val = 0x34;

		/* GRPPWM register. */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xa, 0x3f);

		/* GRPFREQ register. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xb, 0x1f);
	
		/* LED0 OUT */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xc, 0xff);

		/* Red LED only, full on. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x2, 0x0); // red
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x3, 0x0); // green
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x4, 0xff); // blue

	} else {
		val = 0x14;
	
		/* LED0 OUT */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xc, 0xaa);

		/* White LED, full on. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x2, 0xff); // red
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x3, 0xff); // green
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x4, 0xff); // blue
	}
		
	/* Mode 2 Select register */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x1, val);

	/* Select pin 6 of mux to be output. */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x1c, 0x3, 0xbf);

	/* Drive signal FORCE_PWM_N low (turn on fan PWM full on). */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x1c, 0x1, 0x0);
		
	vc_deregister_i2c_gpio(&bb_i2c1);

	setup_soc_gpios(&gpio_map);
}

/**
 * /brief mainboard call for setup that needs to be done after fsp init
 *
 */
void late_mainboard_romstage_entry(void)
{
	u8 val, reset_pressed = 0;

	struct i2c_gpio_dev bb_i2c1 = {
          .sdl_pin = 11,
          .scl_pin = 12,
	};

	if (vc_read_reset_button_level() == 0) 
		reset_pressed = 1;

	vc_register_i2c_gpio(&bb_i2c1);

	/* Mode1 Select register  */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x0, 0x1);

	/* LED1 OUT (off) */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xd, 0x0);

	if (reset_pressed || (cmos_read(0x10) & 1)) {

		val = 0x34;

		cmos_write(cmos_read(0x10) | 1, 0x10);

		/* GRPPWM register. */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xa, 0x3f);

		/* GRPFREQ register. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xb, 0x1f);
	
		/* LED0 OUT */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xc, 0xff);

		/* Red LED only, full on. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x2, 0x0); // red
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x3, 0x0); // green
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x4, 0xff); // blue

	} else {
		val = 0x14;
	
		/* LED0 OUT */
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xc, 0xaa);

		/* White LED, full on. */	
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x2, 0xff); // red
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x3, 0xff); // green
		vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x4, 0xff); // blue
	}
		
	/* Mode 2 Select register */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x1, val);

	vc_deregister_i2c_gpio(&bb_i2c1);

	interrupt_routing_config();
}

/**
 * Get function disables - most of these will be done automatically
 * @param fd_mask
 */
void get_func_disables(uint32_t *mask)
{

}

void romstage_fsp_rt_buffer_callback(FSP_INIT_RT_BUFFER *FspRtBuffer)
{
	/* No overrides needed */
	return;
}
