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
#include <delay.h>
#include <tpm.h>
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
	//int i;
	struct i2c_gpio_dev bb_i2c1 = {
          .sdl_pin = 11,
          .scl_pin = 12,
	};

	u8 buf[] = {0xff, 0xff, 0xff};
	u8 cmd_buf[] = {0, 0};

	console_init();

	vc_register_i2c_gpio(&bb_i2c1);

	cmd_buf[0] = 0x2;
	cmd_buf[1] = 0x8c;
	/* SMBUS IO Expander addr=0x74, cmd 0x2 (register 0 output bits), bit 2,3,7N high. */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);

	cmd_buf[0] = 0x4;
	cmd_buf[1] = 0;	
	/* SMBUS IO Expander addr=0x74, cmd 0x4 (register 0 no polarity inv). */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);

	cmd_buf[0] = 0x6;
	cmd_buf[1] = 0x43;
	/* SMBUS IO Expander addr=0x74, cmd 0x6 (register 0 output), bit 2,3,4,5,7. */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);

	vc_early_udelay(10000);

	cmd_buf[0] = 0x2;
	cmd_buf[1] = 0xc;
	/* SMBUS IO Expander addr=0x74, cmd 0x2 (register 0 output bits), bit 2,3 high. */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);
	
	vc_early_udelay(10000);

	cmd_buf[0] = 0x2;
	cmd_buf[1] = 0x8c;
	/* SMBUS IO Expander addr=0x74, cmd 0x2 (register 0 output bits), bit 2,3,7N high. */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);

	vc_early_udelay(40000);

	cmd_buf[0] = 0x2;
	cmd_buf[1] = 0xbc;
	/* SMBUS IO Expander addr=0x74, cmd 0x2 (register 0 output bits), bit 2,3,4,5,7N high. */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x74, 2, cmd_buf);

	/* Write PIC for all white LED (RGB: 0xff/0xff/0xff) */
	(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x20, 3, buf);

	vc_deregister_i2c_gpio(&bb_i2c1);

	setup_soc_gpios(&gpio_map);
}

/**
 * /brief mainboard call for setup that needs to be done after fsp init
 *
 */
void late_mainboard_romstage_entry(void)
{
	interrupt_routing_config();
	u8 cntr_buf[] = {0 , 0};
	u8 blue_buf[] = {0x0, 0x0, 0xff};
	u16 cntr_tick1, cntr_tick2;

	struct i2c_gpio_dev bb_i2c1 = {
          .sdl_pin = 11,
          .scl_pin = 12,
	};

	vc_register_i2c_gpio(&bb_i2c1);

	(void) vc_i2c_gpio_block_read_data(&bb_i2c1, 0x2a, 2, cntr_buf);
	cntr_tick1 = (cntr_buf[1] << 8 | cntr_buf[0]);

	vc_early_udelay(10000);

	cntr_buf[0] = 0;
	cntr_buf[1] = 0;
	(void) vc_i2c_gpio_block_read_data(&bb_i2c1, 0x2a, 2, cntr_buf);
	cntr_tick2 = (cntr_buf[1] << 8 | cntr_buf[0]);

	if ((cntr_tick2 - cntr_tick1 > 0) || (cmos_read(0x10) & 1)) {

		if (cntr_tick2 - cntr_tick1 > 0) {

			vc_early_udelay(3000000);

			cntr_buf[0] = 0;
			cntr_buf[1] = 0;
			(void) vc_i2c_gpio_block_read_data(&bb_i2c1, 0x2a, 2, cntr_buf);

			cntr_tick1 = (cntr_buf[1] << 8 | cntr_buf[0]);

			if (cntr_tick1 - cntr_tick2 > 0) {
				(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x20, 3, blue_buf);
				cmos_write(cmos_read(0x10) | 1, 0x10);
			}
		} else {
			(void) vc_i2c_gpio_block_write_data(&bb_i2c1, 0x20, 3, blue_buf);
		}
	}
	vc_deregister_i2c_gpio(&bb_i2c1);

	interrupt_routing_config();

        init_tpm(0);
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
