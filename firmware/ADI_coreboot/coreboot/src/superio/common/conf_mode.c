/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Nico Huber <nico.h@gmx.de>
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

#include <arch/io.h>
#include <device/device.h>
#include <superio/conf_mode.h>

/* Common enter/exit implementations */
#ifdef __PRE_RAM__
#define SIO_PORT_ADDR (dev >> 8)
#else
#define SIO_PORT_ADDR (dev->path.pnp.port)
#endif

void pnp_enter_conf_mode_55(device_t dev)
{
	outb(0x55, SIO_PORT_ADDR);
}

void pnp_enter_conf_mode_8787(device_t dev)
{
	outb(0x87, SIO_PORT_ADDR);
	outb(0x87, SIO_PORT_ADDR);
}
void pnp_enter_conf_mode_7777(device_t dev)
{
	outb(0x77, SIO_PORT_ADDR);
	outb(0x77, SIO_PORT_ADDR);
}

void pnp_exit_conf_mode_aa(device_t dev)
{
	outb(0xaa, SIO_PORT_ADDR);
}

void pnp_enter_conf_mode_870155aa(device_t dev)
{
	outb(0x87, SIO_PORT_ADDR);
	outb(0x01, SIO_PORT_ADDR);
	outb(0x55, SIO_PORT_ADDR);

	if (SIO_PORT_ADDR == 0x4e)
		outb(0xaa, SIO_PORT_ADDR);
	else
		outb(0x55, SIO_PORT_ADDR);
}

void pnp_exit_conf_mode_0202(device_t dev)
{
	outb(0x02, SIO_PORT_ADDR);
	outb(0x02, SIO_PORT_ADDR + 1);
}

#ifndef __PRE_RAM__
const struct pnp_mode_ops pnp_conf_mode_55_aa = {
	.enter_conf_mode = pnp_enter_conf_mode_55,
	.exit_conf_mode  = pnp_exit_conf_mode_aa,
};

const struct pnp_mode_ops pnp_conf_mode_8787_aa = {
	.enter_conf_mode = pnp_enter_conf_mode_8787,
	.exit_conf_mode  = pnp_exit_conf_mode_aa,
};

const struct pnp_mode_ops pnp_conf_mode_7777_aa = {
	.enter_conf_mode = pnp_enter_conf_mode_7777,
	.exit_conf_mode  = pnp_exit_conf_mode_aa
};

const struct pnp_mode_ops pnp_conf_mode_870155_aa = {
	.enter_conf_mode = pnp_enter_conf_mode_870155aa,
	.exit_conf_mode  = pnp_exit_conf_mode_0202,
};
#endif
