/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <console/console.h>
#include <device/device.h>
#include <device/path.h>
#include <device/smbus.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include <arch/io.h>
#include "soc.h"
#include "smbus.h"
#include "cpu/x86/msr.h"

static int lsmbus_read_byte(device_t dev, u8 address)
{
	u16 device;
	struct resource *res;
	struct bus *pbus;

	device = dev->path.i2c.device;
	pbus = get_pbus_smbus(dev);
	res = find_resource(pbus->dev, 0x20);

	return do_smbus_read_byte(res->base, device, address);
}

static int lsmbus_write_byte(device_t dev, u8 address, u8 val)
{
	u16 device;
	struct resource *res;
	struct bus *pbus;

	device = dev->path.i2c.device;
	pbus = get_pbus_smbus(dev);

	res = find_resource(pbus->dev, 0x20);

	return do_smbus_write_byte(res->base, device, address, val);
}

/* fan controller at device 0x2f, fan1 speed address 0x40, fan2 speed ox80 */
int smbus_set_fanSpeed(unsigned int fan_number, unsigned int fan_speed)
{
	unsigned base, val, address, device;


	if((fan_number !=1) && (fan_number != 2))
		return -1;

	if(fan_number ==1)
		address = 0x40;
	else if(fan_number ==2)
		address =  0x80;
	else
		return -1;
	/* SMBUS should already been enabled by fsp */	

	base = SMBUS_IO_BASE;
	device = 0x2f;
	
	
	/* test code, read first */
	val = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "read address 0x%x: 0x%x\n",address, val);

	/* now program the new speed*/
	val = fan_speed;
	return do_smbus_write_byte(base, device, address, val);

	
}


static unsigned char pwm_lut[8][5] = {
	{0x40,0x10,0x10,0x10,0x10},
	{0x40,0x20,0x20,0x20,0x20},
	{0x80,0x28,0x28,0x28,0x28},
	{0xb0,0x30,0x30,0x30,0x30},
	{0xd0,0x40,0x40,0x40,0x40},
	{0xf0,0x50,0x50,0x50,0x50},
	{0xff,0x60,0x60,0x60,0x60},
	{0xff,0x70,0x70,0x70,0x70}

};

static unsigned char pwm_lut_addr[8][5] = {
	{0x51,0x52,0x53,0x54,0x55},
	{0x56,0x57,0x58,0x59,0x5a},
	{0x5b,0x5c,0x5d,0x5e,0x5f},
	{0x60,0x61,0x62,0x63,0x64},
	{0x65,0x66,0x67,0x68,0x69},
	{0x6a,0x6b,0x6c,0x6d,0x6e},
	{0x6f,0x70,0x71,0x72,0x73},
	{0x74,0x75,0x76,0x77,0x78}

};


int smbus_fan_control(void)
{
	int ret=0;
	unsigned int base, device;
	unsigned char address, val;
	int i,j, reg;

	base = SMBUS_IO_BASE;
	device = 0x2f;

	/* test if devuce exist , read ID first */
	address = 0xfe; /* ID register */
	reg = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "read address 0x%x: 0x%x\n",address, reg);

	if((reg <0) || (reg !=0x5d))
		return -1;



/* # set PWM frequency
 0x5e , 0x2b , 00*/
	address = 0x2b;
	val = 0x0;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

/* #unlock LUT table
 0x5e , 0x50 , 0x08*/

	address = 0x50;
	val = 0x08;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

/* set up LUT */
	for (i=0; i<8; i++) 
	{
		for(j=0; j<5 ; j++) 
		{
			address = pwm_lut_addr[i][j];
			val = pwm_lut[i][j];
			ret = do_smbus_write_byte(base, device, address, val);
			if (ret)
			{
	 			 printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  			return ret;
			}


		}

	}


/* set hystersis */
	address = 0x79;
	val = 0x7;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}


/* #activate
 0x5e , 0x50 , 0x30 */
	address = 0x50;
	val = 0x30;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}



	

 	return ret;


}



static struct smbus_bus_operations lops_smbus_bus = {
	.read_byte	= lsmbus_read_byte,
	.write_byte	= lsmbus_write_byte,
};


int smbus_enable_usb(void)
{
	/* unsigned base, val, address, device; */
	int ret=0;
#if 0 /* need driver for SMbus1 */	
	base = SMBUS_IO_BASE;
	device = 0x74;


	address = 0;
	/* test code, read first */
	val = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "SMBUS read device 0x74, address 0x0: 0x%x\n", val);

	address = 0x1;
	/* test code, read first */
	val = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "SMBUS read device 0x74, address 0x1: 0x%x\n", val);

	/* SMBUS should already been enabled by fsp */	
        /* set direction - 0xe8 0x06 0x43 0x60 Write to set value: 0xe8 0x02 0xff 0x7f */

	address = 0x6;
	val = 0x43;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

	address = 0x7;
	val = 0x60;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

	address = 0x2;
	val = 0xff;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

	address = 0x3;
	val = 0x7f;
	ret = do_smbus_write_byte(base, device, address, val);
	if (ret)
	{
	  printk(BIOS_DEBUG, "SMBUS write error addres= 0x%x, val=0x%x, ret=%d\n",address, val, ret);	
	  return ret;
	}

#endif
	return ret;

	
}


static void smbus_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

static struct pci_operations smbus_pci_ops = {
	.set_subsystem    = smbus_set_subsystem,
};

static void rangeley_smbus_read_resources(device_t dev)
{
	struct resource *res;

	/*
	 * The SMBus has two BARS.
	 * BAR0 - MMIO, not used at boot time
	 * BAR4 - IO, Used to talk to the SMBUS during boot, so we maintain
	 * the default setting in the resource allocator.
	 */

	res = pci_get_resource(dev, PCI_BASE_ADDRESS_0);

	res = new_resource(dev, PCI_BASE_ADDRESS_4);
	res->base = SMBUS_IO_BASE;
	res->size = 32;
	res->flags = IORESOURCE_IO | IORESOURCE_FIXED | IORESOURCE_RESERVE |
		     IORESOURCE_STORED | IORESOURCE_ASSIGNED;

}

static struct device_operations smbus_ops = {
	.read_resources		= rangeley_smbus_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init			= 0,
	.scan_bus		= scan_static_bus,
	.ops_smbus_bus		= &lops_smbus_bus,
	.ops_pci		= &smbus_pci_ops,
};

static const struct pci_driver rangeley_smbus __pci_driver = {
	.ops	= &smbus_ops,
	.vendor	= PCI_VENDOR_ID_INTEL,
	.device	= 0x1F3C,
};
