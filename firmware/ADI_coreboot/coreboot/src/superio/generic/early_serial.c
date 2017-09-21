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
#include <device/pnp_def.h>
#include "superio.h"
#include <platform_sio_config.h>

static void pnp_enter_conf_state(device_t dev)
{
	u16 port = dev >> 8;

#if IS_ENABLED(PLATFORM_LOCK_8787AA)
	outb(0x87, port);
	outb(0x87, port);
#elif IS_ENABLED(PLATFORM_LOCK_7777AA)
	outb(0x77, port);
	outb(0x77, port);
#elif IS_ENABLED(PLATFORM_LOCK_55AA)
	outb(0x55, port);
#endif

}

static void pnp_exit_conf_state(device_t dev)
{
	u16 port = dev >> 8;
	outb(0xaa, port);
}

/* set input clock */
static void configure_uart_clk(device_t dev)
{
	pnp_write_config(dev, SIO_CLK_SEL_REG,
	                 SIO_CLOCK_SPEED_VALUE);
}

void enable_serial_early(device_t dev, u16 iobase)
{
#if PLATFORM_USE_UNLOCK
	pnp_enter_conf_state(dev);
#endif
	if (PLATFORM_INIT_UART_CLOCK_SPEED)
		configure_uart_clk(dev);

	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, iobase);
	pnp_set_enable(dev, 1);
#if PLATFORM_USE_UNLOCK
	pnp_exit_conf_state(dev);
#endif
}
