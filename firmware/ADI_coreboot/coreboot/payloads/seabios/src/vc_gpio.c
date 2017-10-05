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

#include <stdint.h>
#include <string.h>
#include "vc_gpio.h"
#include "hw/pci.h"
#include "hw/pci_ids.h"
#include "hw/pci_regs.h"

#include "x86.h"

/* GPIOBASE */
#define GPIO_SC_USE_SEL  0x00
#define GPIO_SC_IO_SEL   0x04
#define GPIO_SC_GP_LVL   0x08

static inline __attribute__((always_inline))
void vc_early_udelay(const uint32_t delay_us)
{
	uint32_t i;

	for (i = 0; i < delay_us; i++) 
        	inb(0x80);
}


void vc_register_i2c_gpio(struct i2c_gpio_dev *dev)
{
	u16 bdf;
	u32 scl_pin = (1 << dev->scl_pin);
	
	/* Encode the LPC BDF */
	bdf = pci_to_bdf(0x0, 0x1f, 0);

	dev->gpiobase = (pci_config_readw(bdf, 0x48) & ~0xf);

        /* Read default USE_SEL for native/GPIO mode. */
        dev->cached_io_mode = inl(dev->gpiobase + GPIO_SC_USE_SEL);

        /* Set GPIO mode for all pins. */
        outl(0xffffffff, dev->gpiobase + GPIO_SC_USE_SEL);

        /* Read default IO_SEL for directional input/output. */
        dev->cached_io_sel = inl(dev->gpiobase + GPIO_SC_IO_SEL);
	dev->active_io_sel = dev->cached_io_sel;

        /* Set direction to 'in'. */
        outl(0xffffffff, dev->gpiobase + GPIO_SC_IO_SEL);

        /* Read what is on the GPIO line. */
        dev->cached_io_lvl = inl(dev->gpiobase + GPIO_SC_GP_LVL);
	dev->active_io_lvl = dev->cached_io_lvl;

        /* Make pins GPIO 8/9 and default the rest. */
        outl(dev->cached_io_mode | scl_pin |(1 << dev->sdl_pin),
                               dev->gpiobase + GPIO_SC_USE_SEL);
        
	dev->active_io_sel &= ~scl_pin;
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	dev->active_io_lvl &= ~scl_pin;
	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);

	dev->active_io_sel |= scl_pin;
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
	
	vc_early_udelay(100);
}

void vc_deregister_i2c_gpio(struct i2c_gpio_dev *dev)
{
	/* Make GP(O) to set level. */
	dev->active_io_sel &= ~((1 << dev->scl_pin) | (1 << dev->sdl_pin));
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	/* Set output to high. */
	dev->active_io_lvl |= ((1 << dev->scl_pin) | (1 << dev->sdl_pin));
	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);

	/* Program as a GP(I). */	
	dev->active_io_sel |= ((1 << dev->scl_pin) | (1 << dev->sdl_pin));
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	dev->cached_io_mode &= ~((1 << dev->scl_pin) | (1 << dev->sdl_pin));
        outl(dev->cached_io_mode, dev->gpiobase + GPIO_SC_USE_SEL);
}

static inline void vc_i2c_gpio_ack(struct i2c_gpio_dev *dev)
{
	u32 sdl_pin = (1 << dev->sdl_pin);

	dev->active_io_sel &= ~sdl_pin;
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	dev->active_io_lvl &= ~sdl_pin;
	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);
}

static void vc_i2c_gpio_stop_condition(struct i2c_gpio_dev *dev)
{
	/* Complete our transaction by driving clock low. */
	vc_early_udelay(2);
	dev->active_io_sel &= ~(1 << dev->scl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
	vc_early_udelay(2);
	
	/* Stop condition by clock rising first. */
	dev->active_io_sel |= (1 << dev->scl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	vc_early_udelay(6);

	/* the data line rising second. */
	dev->active_io_lvl |= (1 << dev->sdl_pin);
	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);
}

static inline void vc_i2c_gpio_start_condition(struct i2c_gpio_dev *dev)
{
	u32 sdl_pin = (1 << dev->sdl_pin);
	
        /* Set SDL to output and default the rest. */
        dev->active_io_sel &= ~sdl_pin;
        outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	/* I2C clock start condition. */
	dev->active_io_lvl |= sdl_pin;
        outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);

	vc_early_udelay(6);

	/* I2C_START condition. */
	dev->active_io_lvl &= ~sdl_pin;
	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);

	vc_early_udelay(5);
}


inline int vc_i2c_gpio_write_byte(struct i2c_gpio_dev *dev, u8 addr, u8 cmd, u8 data)
{
	int i = 0, err = 0, j, val;
#define I2C_WRITE_CMD	3
#define I2C_WRITE_BYTE	8

	/* We send 8 bits including R/W. */
	addr = ((addr & 0x7f) << 1);

	/* Plus our write operation. */
	addr &= ~1;

	vc_i2c_gpio_start_condition(dev);

	while (likely (i < I2C_WRITE_CMD)) {

		if (i == 0)
			val = addr;
		if (i == 1)
			val = cmd;
		if (i == 2)
			val = data;
		j = 0;

		while (likely (j < I2C_WRITE_BYTE)) {

			/* Write clock line low. */
                        dev->active_io_sel &= ~(1 << dev->scl_pin);

                        outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

			vc_early_udelay(1);

			dev->active_io_sel &= ~(1 << dev->sdl_pin);

			outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
			
                       	if ((val & 0x80))
				dev->active_io_lvl |= (1 << dev->sdl_pin);
			else
				dev->active_io_lvl &= ~(1 << dev->sdl_pin);

			outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);

			vc_early_udelay(2);

			/* Write clock line high. */
                        dev->active_io_sel |= (1 << dev->scl_pin);
                        outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
			vc_early_udelay(1);
			
			val = val << 1;
			
			j++;
		}
                        
		dev->active_io_sel &= ~(1 << dev->scl_pin);
                outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

		vc_i2c_gpio_ack(dev);
	
		vc_early_udelay(3);
		
		/* Write clock line high. */
                dev->active_io_sel |= (1 << dev->scl_pin);
                outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
		vc_early_udelay(3);

		i++;
	}

	vc_i2c_gpio_stop_condition(dev);

	return err;
}

void vc_led_blink_red(void)
{
	u8 val;

	struct i2c_gpio_dev bb_i2c1 = {
          .sdl_pin = 11,
          .scl_pin = 12,
	};

	vc_register_i2c_gpio(&bb_i2c1);

	/* Mode1 Select register  */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x0, 0x1);

	/* LED1 OUT (off) */
	(void) vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xd, 0x0);

	val = 0x34;

	/* GRPPWM register. */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xa, 0x3f);

	/* GRPFREQ register. */	
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xb, 0x1f);
	
	/* LED0 OUT */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0xc, 0xff);

	/* Red LED only, full on. */	
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x2, 0xff); // red
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x3, 0x0); // green
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x4, 0x00); // blue

	/* Mode 2 Select register */
	vc_i2c_gpio_write_byte(&bb_i2c1, 0x54, 0x1, val);

	vc_deregister_i2c_gpio(&bb_i2c1);
}

