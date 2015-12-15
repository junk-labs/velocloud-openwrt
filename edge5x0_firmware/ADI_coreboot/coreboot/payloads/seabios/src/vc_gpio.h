/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
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

#ifndef VC_GPIO_H
#define VC_GPIO_H

struct i2c_gpio_dev {
	u8 scl_pin;
	u8 sdl_pin;
	u16 gpiobase;
	u32 active_io_lvl;
	u32 cached_io_lvl;
	u32 active_io_sel;
	u32 cached_io_sel;
	u32 cached_io_mode;
};

void vc_register_i2c_gpio(struct i2c_gpio_dev *dev);

void vc_deregister_i2c_gpio(struct i2c_gpio_dev *dev);

int vc_i2c_gpio_write_byte(struct i2c_gpio_dev *dev, u8 addr, u8 cmd, u8 data);

void vc_led_blink_red(void);

#endif
