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
#include <arch/io.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_def.h>
#include <device/pnp_def.h>
#include <console/console.h>

#include "soc.h"
#include "gpio.h"

#define MAX_GPIO_NUMBER 31 /* zero based */

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

static inline __attribute__((always_inline))
uint16_t pci_io_read_config16_simple(uint32_t dev, unsigned where)
{
        unsigned addr;
#if !CONFIG_PCI_IO_CFG_EXT
        addr = (dev>>4) | where;
#else
        addr = (dev>>4) | (where & 0xff) | ((where & 0xf00)<<16);
#endif
        outl(0x80000000 | (addr & ~3), 0xCF8);
        return inw(0xCFC + (addr & 2));
}

static inline __attribute__((always_inline))
uint32_t pci_io_read_config32_simple(uint32_t dev, unsigned where)
{
        unsigned addr;
#if !CONFIG_PCI_IO_CFG_EXT
        addr = (dev>>4) | where;
#else
        addr = (dev>>4) | (where & 0xff) | ((where & 0xf00)<<16);
#endif
        outl(0x80000000 | (addr & ~3), 0xCF8);
        return inl(0xCFC);
}

void vc_early_udelay(const uint32_t delay_us)
{
	uint32_t i;

	for (i = 0; i < delay_us; i++) 
        	inb(0x80);
}


uint32_t vc_read_reset_button_level(void)
{
	u16 gpiobase;
	u32 rst_btn_n = (1 << 18);
	u32 rc, io_mode, io_sel;
	
#ifdef __SIMPLE_DEVICE__
        gpiobase = pci_read_config16(SOC_LPC_DEV, GBASE) & ~0xf;
#else
        gpiobase = pci_io_read_config16_simple(SOC_LPC_DEV, GBASE) & ~0xf;
#endif /* __SIMPLE_DEVICE__ */

        /* Read default USE_SEL for native/GPIO mode. */
        io_mode = inl(gpiobase + GPIO_SC_USE_SEL);

        /* Set GPIO mode for rst_btn pin. */
        outl(io_mode | rst_btn_n, gpiobase + GPIO_SC_USE_SEL);
        
	/* Read default IO_SEL for directional input/output. */
        io_sel = inl(gpiobase + GPIO_SC_IO_SEL);

        /* Set direction to 'in' for rst_btn_pin. */
        outl(io_sel | rst_btn_n, gpiobase + GPIO_SC_IO_SEL);

	/* Read current level */
	rc = inl(gpiobase + GPIO_SC_GP_LVL) & rst_btn_n;

	outl(io_sel, gpiobase + GPIO_SC_USE_SEL);

        outl(io_mode, gpiobase + GPIO_SC_USE_SEL);

	return (rc);
} 


void vc_register_i2c_gpio(struct i2c_gpio_dev *dev)
{
	u32 scl_pin = (1 << dev->scl_pin);

#ifdef __SIMPLE_DEVICE__
        dev->gpiobase = pci_read_config16(SOC_LPC_DEV, GBASE) & ~0xf;
#else
        dev->gpiobase = pci_io_read_config16_simple(SOC_LPC_DEV, GBASE) & ~0xf;
#endif /* __SIMPLE_DEVICE__ */

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
	//outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);
	outl(dev->cached_io_lvl | (1 << dev->scl_pin) | (1 << dev->sdl_pin), dev->gpiobase + GPIO_SC_GP_LVL);

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

static inline void vc_i2c_gpio_new_ack(struct i2c_gpio_dev *dev)
{
	u32 sdl_pin = (1 << dev->sdl_pin);

	dev->active_io_sel |= sdl_pin;
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	vc_early_udelay(2);

	dev->active_io_sel &= ~sdl_pin;
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

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

	vc_early_udelay(10);
}

static void vc_i2c_gpio_new_stop_condition(struct i2c_gpio_dev *dev)
{
	/* Complete our transaction by driving clock low. */
	vc_early_udelay(2);

	/* Stop condition by clock rising first. */
	dev->active_io_sel |= (1 << dev->scl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	vc_early_udelay(6);

	/* SDL pin to input. */
	dev->active_io_sel |= (1 << dev->sdl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	vc_early_udelay(5);
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

static inline void vc_i2c_gpio_new_start_condition(struct i2c_gpio_dev *dev)
{
	u32 sdl_pin = (1 << dev->sdl_pin);
	
        /* Set SDL to output and default the rest. */
	if ((dev->active_io_sel & sdl_pin)) {
        	dev->active_io_sel &= ~sdl_pin;
        	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

		/* I2C clock start condition. */
		dev->active_io_lvl &= ~sdl_pin;
        	outl(dev->active_io_lvl, dev->gpiobase + GPIO_SC_GP_LVL);
	}

	/* Set SDL to input and default the rest. */ 
	dev->active_io_sel |= sdl_pin;
        outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

	vc_early_udelay(100);

	/* I2C_START condition. */
        /* Set SDL to output and default the rest. */
        dev->active_io_sel &= ~sdl_pin;
        outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
}

static inline void vc_clock_high(struct i2c_gpio_dev *dev)
{
	/* Write clock line high. */
	dev->active_io_sel |= (1 << dev->scl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
}

static inline void vc_clock_low(struct i2c_gpio_dev *dev)
{
	/* Write clock line low. */
	dev->active_io_sel &= ~(1 << dev->scl_pin);
	outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
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

inline int vc_i2c_gpio_block_write_data(struct i2c_gpio_dev *dev, u8 addr, u8 datalen, u8 *data)
{
	int i = 0, err = 0, j, k = 0;
	volatile int val;

#define I2C_WRITE_ADDR	1
#define I2C_WRITE_BYTE	8

	/* We send 8 bits including R/W. */
	addr = ((addr & 0x7f) << 1);

	/* Plus our write operation. */
	addr &= ~1;

	vc_i2c_gpio_new_start_condition(dev);

	while (likely (i < (I2C_WRITE_ADDR + datalen))) {

		if (i == 0)
			val = addr;
		if (i >= 1) {
			val = data[k];
			k++;
		}

		for (j = 0; j < I2C_WRITE_BYTE; j++) {

			/* Write clock line low. */
			vc_clock_low(dev);
			vc_early_udelay(1);

			if ((val & 0x80))
				dev->active_io_sel |= (1 << dev->sdl_pin);
			else
				dev->active_io_sel &= ~(1 << dev->sdl_pin);

			outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
			vc_early_udelay(2);

			/* Write clock line high. */
			vc_clock_high(dev);
			vc_early_udelay(1);

			val = val << 1;
		}
		vc_clock_low(dev);

		/* Set data line low (output). */
		dev->active_io_sel &= ~(1 << dev->sdl_pin);
		outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

		vc_early_udelay(20);

		vc_clock_high(dev);

		vc_early_udelay(2);

		vc_clock_low(dev);

		vc_early_udelay(40);

		i++;
	}

	vc_i2c_gpio_new_stop_condition(dev);
	return err;
}

inline int vc_i2c_gpio_block_read_data(struct i2c_gpio_dev *dev, u8 addr, u8 datalen, u8 *data)
{
	int i = 0, err = 0, j, k = 0;
	volatile int val;
	int release = 0;

#define I2C_READ_ADDR	1
#define I2C_READ_BYTE	8

	/* We send 8 bits including R/W and read op */
	val = ((addr & 0x7f) << 1) | 1;

	vc_i2c_gpio_new_start_condition(dev);

	while (likely (i < (I2C_READ_ADDR + datalen))) {

		for (j = 0; j < I2C_READ_BYTE; j++) {

			/* Write clock line low. */
			vc_clock_low(dev);

			vc_early_udelay(10);

			//release = 0;
			if (i == 0) {
				if ((val & 0x80)) {
					dev->active_io_sel |= (1 << dev->sdl_pin);
					outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
				} else {
					dev->active_io_sel &= ~(1 << dev->sdl_pin);
					outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
				}
				val = val << 1;
			} else {
				data[k] = (data[k] << 1);

				if (! release) {
					release = 1;

					/* Set data line high (input). */
					dev->active_io_sel |= (1 << dev->sdl_pin);

					/* Set direction to 'in' for data. */
					outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
				}
				/* Read current level */
				data[k] |= ((inl(dev->gpiobase + GPIO_SC_GP_LVL) >> dev->sdl_pin) & 1);
			}
			vc_early_udelay(20);

			/* Write clock line high. */
			vc_clock_high(dev);

			vc_early_udelay(10);
		}
		vc_clock_low(dev);

		/* Set data line low (output). */
		dev->active_io_sel &= ~(1 << dev->sdl_pin);
		outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);

		vc_early_udelay(20);

		if (i == datalen) {
		/* Set data line high (input). */
		dev->active_io_sel |= (1 << dev->sdl_pin);
		outl(dev->active_io_sel, dev->gpiobase + GPIO_SC_IO_SEL);
		}
		vc_clock_high(dev);

		vc_early_udelay(10);

		vc_clock_low(dev);

		vc_early_udelay(40);

		i++;

		if (i > 1) { k++; }
	}
	vc_i2c_gpio_new_stop_condition(dev);

	return err;
}

void setup_soc_gpios(const struct soc_gpio_map *gpio)
{
#ifdef __SIMPLE_DEVICE__
	u16 gpiobase = pci_read_config16(SOC_LPC_DEV, GBASE) & ~0xf;
	u32 cfiobase = pci_read_config32(SOC_LPC_DEV, IOBASE) & ~0xf;
#else
	u16 gpiobase = pci_io_read_config16_simple(SOC_LPC_DEV, GBASE) & ~0xf;
        u32 cfiobase = pci_io_read_config32_simple(SOC_LPC_DEV, IOBASE) & ~0xf;	

#endif /* __SIMPLE_DEVICE__ */

	u32 reg, cfio_cnt = 0;


	/* GPIO */
	if (gpio->core.level)
		outl(*((u32*)gpio->core.level), gpiobase + GPIO_SC_GP_LVL);
	if (gpio->core.mode)
		outl(*((u32*)gpio->core.mode), gpiobase + GPIO_SC_USE_SEL);
	if (gpio->core.direction)
		outl(*((u32*)gpio->core.direction), gpiobase + GPIO_SC_IO_SEL);
	if (gpio->core.tpe)
		outl(*((u32*)gpio->core.tpe), gpiobase + GPIO_SC_TPE);
	if (gpio->core.tne)
		outl(*((u32*)gpio->core.tne), gpiobase + GPIO_SC_TNE);
	if (gpio->core.ts)
		outl(*((u32*)gpio->core.ts), gpiobase + GPIO_SC_TS);

	/* GPIO SUS Well Set 1 */
	if (gpio->sus.level)
		outl(*((u32*)gpio->sus.level), gpiobase + GPIO_SUS_GP_LVL);
	if (gpio->sus.mode)
		outl(*((u32*)gpio->sus.mode), gpiobase + GPIO_SUS_USE_SEL);
	if (gpio->sus.direction)
		outl(*((u32*)gpio->sus.direction), gpiobase + GPIO_SUS_IO_SEL);
	if (gpio->sus.tpe)
		outl(*((u32*)gpio->sus.tpe), gpiobase + GPIO_SUS_TPE);
	if (gpio->sus.tne)
		outl(*((u32*)gpio->sus.tne), gpiobase + GPIO_SUS_TNE);
	if (gpio->sus.ts)
		outl(*((u32*)gpio->sus.ts), gpiobase + GPIO_SUS_TS);
	if (gpio->sus.we)
		outl(*((u32*)gpio->sus.we), gpiobase + GPIO_SUS_WE);

	/* GPIO PAD settings */
	/* CFIO Core Well Set 1 */
	if ((gpio->core.cfio_init != NULL) || (gpio->core.cfio_entrynum != 0)) {
		write32(cfiobase + 0x0700, (u32)0x01001002);
		for(cfio_cnt = 0; cfio_cnt < gpio->core.cfio_entrynum; cfio_cnt++) {
			if (!((u32)gpio->core.cfio_init[cfio_cnt].pad_conf_0))
				continue;
			write32(cfiobase + CFIO_PAD_CONF0 + (16*cfio_cnt), (u32)gpio->core.cfio_init[cfio_cnt].pad_conf_0);
			write32(cfiobase + CFIO_PAD_CONF1 + (16*cfio_cnt), (u32)gpio->core.cfio_init[cfio_cnt].pad_conf_1);
			write32(cfiobase + CFIO_PAD_VAL + (16*cfio_cnt), (u32)gpio->core.cfio_init[cfio_cnt].pad_val);
			write32(cfiobase + CFIO_PAD_DFT + (16*cfio_cnt), (u32)gpio->core.cfio_init[cfio_cnt].pad_dft);
		}
		write32(cfiobase + 0x0700, (u32)0x01041002);
	}
	
	/* configure GPIO 13 and 14 for uaart 0, change Pin function to 0x2 */
	reg = read32(cfiobase + CFIO_UART0_RXD);
	reg = (reg & 0xffffff8) | 0x2;
	write32(cfiobase + CFIO_UART0_RXD,reg);

	reg = read32(cfiobase + CFIO_UART0_TXD);
	reg = (reg & 0xffffff8) | 0x2;

	/* CFIO SUS Well Set 1 */
	if ((gpio->sus.cfio_init != NULL) || (gpio->sus.cfio_entrynum != 0)) {
		write32(cfiobase + 0x1700, (u32)0x01001002);
		for(cfio_cnt = 0; cfio_cnt < gpio->sus.cfio_entrynum; cfio_cnt++) {
			if (!((u32)gpio->sus.cfio_init[cfio_cnt].pad_conf_0))
				continue;
			write32(cfiobase + CFIO_PAD_CONF0 + 0x1000 + (16*cfio_cnt), (u32)gpio->sus.cfio_init[cfio_cnt].pad_conf_0);
			write32(cfiobase + CFIO_PAD_CONF1 + 0x1000 + (16*cfio_cnt), (u32)gpio->sus.cfio_init[cfio_cnt].pad_conf_1);
			write32(cfiobase + CFIO_PAD_VAL + 0x1000 + (16*cfio_cnt), (u32)gpio->sus.cfio_init[cfio_cnt].pad_val);
			write32(cfiobase + CFIO_PAD_DFT + 0x1000 + (16*cfio_cnt), (u32)gpio->sus.cfio_init[cfio_cnt].pad_dft);
		}
		write32(cfiobase + 0x1700, (u32)0x01041002);
	}
}

int get_gpio(int gpio_num)
{
#ifdef __SIMPLE_DEVICE__
	u16 gpio_base = pci_read_config16(SOC_LPC_DEV, GBASE) & ~0xf;
#else
	 u16 gpio_base = pci_io_read_config16_simple(SOC_LPC_DEV, GBASE) & ~0xf;
#endif
	int bit;

	if (gpio_num > MAX_GPIO_NUMBER)
		return 0; /* Ignore wrong gpio numbers. */

	bit = gpio_num % 32;

	return (inl(gpio_base + GPIO_SC_USE_SEL) >> bit) & 1;
}
