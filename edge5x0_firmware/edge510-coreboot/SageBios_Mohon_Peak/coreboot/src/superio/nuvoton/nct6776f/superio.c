/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (c) 2014 Sage Electronic Engineering, Inc.
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
#include <device/pnp.h>
#include <pc80/keyboard.h>
#include <superio/conf_mode.h>
#include <stdlib.h>
#include "nct6776f.h"
#include "chip.h"

static void nct6776f_init(device_t dev)
{
	struct superio_nuvoton_nct6776f_config *conf = dev->chip_info;
	u8 reg10, reg26;

	if (!dev->enabled)
		return;

	pnp_enter_conf_mode(dev);

	/* Before accessing CR10 OR CR11 Bit 4 in CR26 must be set to 1 */
	reg26 = pnp_read_config(dev, GLOBAL_OPTION_CR26);
	reg26 |= CR26_LOCK_REG;
	pnp_write_config(dev, GLOBAL_OPTION_CR26, reg26);

	switch(dev->path.pnp.device) {
	/* SP1IRQ type selection is controlled by CR 10, bit 5 */
	case NCT6776F_SP1:
		reg10 = pnp_read_config(dev, IRQ_TYPE_SEL_CR10);
		if (conf->irq_trigger_type == IRQ_TRIGGER_LEVEL)
			reg10 |= (1 << SP1_IRQ_TYPE_SELECT_BIT);
		else
			reg10 &= ~(1 << SP1_IRQ_TYPE_SELECT_BIT);
		pnp_write_config(dev, IRQ_TYPE_SEL_CR10, reg10);
		break;

	case NCT6776F_SP2:
	/* SP2 IRQ type selection is controlled by CR 10, bit 4 */

		reg10 = pnp_read_config(dev, IRQ_TYPE_SEL_CR10);
		if (conf->irq_trigger_type == IRQ_TRIGGER_LEVEL)
			reg10 |= (1 << SP2_IRQ_TYPE_SELECT_BIT);
		else
			reg10 &= ~(1 << SP2_IRQ_TYPE_SELECT_BIT);
		pnp_write_config(dev, IRQ_TYPE_SEL_CR10, reg10);
		break;
	case NCT6776F_KBD:
		pc_keyboard_init();
		break;

	default:
		break;
	}

	//Clear access control register
	reg26 = pnp_read_config(dev, GLOBAL_OPTION_CR26);
	reg26 &= ~CR26_LOCK_REG;
	pnp_write_config(dev, GLOBAL_OPTION_CR26, reg26);
	pnp_exit_conf_mode(dev);
}

static struct device_operations ops = {
	.read_resources   = pnp_read_resources,
	.set_resources    = pnp_set_resources,
	.enable_resources = pnp_enable_resources,
	.enable           = pnp_alt_enable,
	.init             = nct6776f_init,
	.ops_pnp_mode     = &pnp_conf_mode_8787_aa,
};

static struct pnp_info pnp_dev_info[] = {
	{ &ops, NCT6776F_FDC,  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0}, },
	{ &ops, NCT6776F_LPT,  PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, {0x07f8, 0}, },

	{ &ops, NCT6776F_SP1,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },
	{ &ops, NCT6776F_SP2,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },

	{ &ops, NCT6776F_KBD,  PNP_IO0 | PNP_IO1 | PNP_IRQ0 | PNP_IRQ1, {0x07ff, 0}, {0x07ff, 4}, },
	{ &ops, NCT6776F_CIR,  PNP_IO0 | PNP_IRQ0, {0x07f8, 0}, },

	{ &ops, NCT6776F_GPIO6789},
	{ &ops, NCT6776F_GPIO_WDT},
	{ &ops, NCT6776F_GPIO2345},
	{ &ops, NCT6776F_ACPI},
	{ &ops, NCT6776F_HWMON},
	{ &ops, NCT6776F_VID},
	{ &ops, NCT6776F_CIR_WAKE},
	{ &ops, NCT6776F_GPIO_PP_OD},
	{ &ops, NCT6776F_SVID},
	{ &ops, NCT6776F_SLEEP},
	{ &ops, NCT6776F_GPIOA},

	};

static void enable_dev(struct device *dev)
{
	pnp_enable_devices(dev, &ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}

struct chip_operations superio_nuvoton_nct5104d_ops = {
	CHIP_NAME("NUVOTON NCT6776(D/F) Super I/O")
	.enable_dev = enable_dev,
};
