/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <arch/io.h>
#include <device/device.h>
#include <device/pnp.h>
#include <superio/conf_mode.h>
#include <console/console.h>
#include <stdlib.h>
#include "chip.h"
#include "f81216ad.h"

static void f81216ad_init(device_t dev)
{
	if (!dev->enabled)
		return;

	switch (dev->path.pnp.device) {
	// not needed for serial or watchdog

	break;
	}
}

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = pnp_set_resources,
	.enable_resources = pnp_enable_resources,
	.enable           = pnp_alt_enable,
	.init             = f81216ad_init,
	.ops_pnp_mode     = &pnp_conf_mode_7777_aa,
};

static struct pnp_info pnp_dev_info[] = {
	{ &ops, F81216AD_SP1,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
	{ &ops, F81216AD_SP2,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
	{ &ops, F81216AD_SP3,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
	{ &ops, F81216AD_SP4,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
	{ &ops, F81216AD_WDT,  PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
};

static void enable_dev(device_t dev)
{
	pnp_enable_devices(dev, &ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}

struct chip_operations superio_fintek_f81216ad_ops = {
	CHIP_NAME("Fintek F81216AD Super I/O")
	.enable_dev = enable_dev
};
