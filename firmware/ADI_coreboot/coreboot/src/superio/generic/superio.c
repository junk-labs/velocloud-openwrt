/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <arch/io.h>
#include <device/device.h>
#include <device/pnp.h>
#include <superio/conf_mode.h>
#include <pc80/keyboard.h>
#include <platform_sio_config.h>
#include <stdlib.h>
#include <console/console.h>
#include "superio.h"
#include "chip.h"

/**
 * Initialize the specified Super I/O device.
 *
 * @param dev Pointer to structure describing a Super I/O device.
 */

static void generic_sio_init(device_t dev)
{
	if (!dev->enabled)
		return;

	switch(dev->path.pnp.device) {
	case PLATFORM_PS2_KBC_LDN:
		printk(BIOS_DEBUG,"Initializing SIO PS2 Keyboard controller device.\n");
		pc_keyboard_init();
		break;
	}
}

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = pnp_set_resources,
	.enable_resources = pnp_enable_resources,

#if IS_ENABLED(PLATFORM_USE_ALT_ENABLE)
	.enable           = pnp_alt_enable,
#else
	.enable           = pnp_enable,
#endif

	.init             = generic_sio_init,

#if IS_ENABLED(PLATFORM_LOCK_8787AA)
	.ops_pnp_mode     = &pnp_conf_mode_8787_aa,
#elif IS_ENABLED(PLATFORM_LOCK_55AA)
	.ops_pnp_mode     = &pnp_conf_mode_55_aa,
#elif IS_ENABLED(PLATFORM_LOCK_7777AA)
	.ops_pnp_mode     = &pnp_conf_mode_7777_aa,
#endif
};


static struct pnp_info pnp_dev_info[] = {

#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL1)
	{ &ops, PLATFORM_SP1_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL2)
	{ &ops, PLATFORM_SP2_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL3)
	{ &ops, PLATFORM_SP3_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL4)
	{ &ops, PLATFORM_SP4_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL5)
	{ &ops, PLATFORM_SP5_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SERIAL6)
	{ &ops, PLATFORM_SP6_LDN,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
#endif

#if IS_ENABLED(PLATFORM_ENABLE_PNP_FDC)
	{ &ops, PLATFORM_FDC_LDN, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0}, },
#endif

#if IS_ENABLED(PLATFORM_ENABLE_PNP_PP1)
	{ &ops, PLATFORM_PP1_LDN  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_PP1)
	{ &ops, PLATFORM_PP2_LDN  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0}, },
#endif

#if IS_ENABLED(PLATFORM_ENABLE_PNP_PS2_KEYBOARD)
	{ &ops, PLATFORM_PS2_KBC_LDN, PNP_IO0 | PNP_IO1 | PNP_IRQ0 | PNP_IRQ1, {0x07ff, 0}, {0x07ff, 4}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_GPIO1)
	{ &ops, PLATFORM_GPIO1_LDN, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_HWM)
	{ &ops, PLATFORM_HWM_LDN,  PNP_IO0 | PNP_IRQ0, {0x0ff8, 0}, },
#endif
#if IS_ENABLED(PLATFORM_ENABLE_PNP_SPI)
	{ &ops, PLATFORM_SPI_LDN, },
#endif
};

/**
 * Create device structures and allocate resources to devices specified in the
 * pnp_dev_info array (above).
 *
 * @param dev Pointer to structure describing a Super I/O device.
 */
static void enable_dev(device_t dev)
{
	pnp_enable_devices(dev, &ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
	printk(BIOS_DEBUG,"generic SIO devices initialized.");
}

struct chip_operations superio_generic_ops = {
	CHIP_NAME(PLATFORM_SIO_CHIP_NAME)
	.enable_dev = enable_dev
};
