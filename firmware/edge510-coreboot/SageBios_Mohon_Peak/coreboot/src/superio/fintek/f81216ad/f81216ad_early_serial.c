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

/* Pre-RAM driver for the Fintek F81216AD Super I/O chip. */

#include <arch/io.h>
#include <device/pnp_def.h>
#include "f81216ad.h"

static void enter_conf_state(device_t dev)
{
	u16 port = dev >> 8;
	outb(0x77, port);
	outb(0x77, port);
}

static void exit_conf_state(device_t dev)
{
	u16 port = dev >> 8;
	outb(0xaa, port);
}

/* set input clock to 48MHz. */
static void f81216ad_configure_uart_clk(device_t dev)
{
	pnp_write_config(dev, F81216AD_CLK_SEL_REG,
	                 F81216AD_48MHZ_CLK);
}

void f81216ad_enable_serial(device_t dev, u16 iobase)
{
	enter_conf_state(dev);
	f81216ad_configure_uart_clk(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, iobase);
	pnp_set_enable(dev, 1);
	exit_conf_state(dev);
}

void f81216ad_disable_wdt(device_t dev)
{
	/* disable WDT */
	enter_conf_state(dev);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	exit_conf_state(dev);

}
