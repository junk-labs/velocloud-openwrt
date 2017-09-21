/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (C) 2014 Sage Electronic Engineering, LLC.
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
#include <superio/conf_mode.h>
#include "nct6776f.h"

void nct6776f_enable_serial(device_t dev, u16 iobase)
{
	pnp_enter_conf_mode_8787(dev);
	/*
	 * Change muxing from GPIO8 to COM1
	 * Change muxing from GPIO1 to COM2
	 */
	//TODO: change the 0x00 to be a devicetree register setting.
	pnp_write_config(dev, GLOBAL_OPTION_CR2A, 0x00);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev,0);
	pnp_set_iobase(dev,PNP_IDX_IO0, iobase);
	pnp_set_enable(dev,1);
	pnp_exit_conf_mode_aa(dev);
}
