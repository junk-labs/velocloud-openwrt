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



static int lsmbus_read_byte(device_t dev, u8 address)
{
	u16 device;
	struct resource *res;
	struct bus *pbus;

	device = dev->path.i2c.device;
	pbus = get_pbus_smbus(dev);
	res = find_resource(pbus->dev, 0x20);
	printk(BIOS_DEBUG,"lsmbus_read_byte()\n");
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


static struct smbus_bus_operations lops_smbus_bus = {
	.read_byte	= lsmbus_read_byte,
	.write_byte	= lsmbus_write_byte,
};


int smbus_enable_usb3_clock(void)
{
	unsigned base, val, address, device;
	
	/* SMBUS should already been enabled by fsp */	

	base = SMBUS_IO_BASE;
	device = 0x69;
	address = 0x82;
	
	/* test code, read first */
	val = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "read address 0x82: 0x%x\n", val);

	/* now write to enable clock */
	val = 0xff;
	return do_smbus_write_byte(base, device, address, val);

	
}

int smbus_enable_spread_spectrum(void)
{
	unsigned base, val, address, device;
	
	/* SMBUS should already been enabled by fsp */	

	base = SMBUS_IO_BASE;
	device = 0x69;
	address = 0x84;
	
	/* test code, read first */
	val = do_smbus_read_byte(base, device, address);
	printk(BIOS_DEBUG, "read address 0x84: 0x%x\n", val);

	/* now enable spread spectrum feature */
	val = 0xfd;
	return do_smbus_write_byte(base, device, address, val);

	
}



static void smbus_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	printk(BIOS_DEBUG, "SMBUS: smbus_set_subsystem\n");
	
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
cd ../	 */

	printk(BIOS_DEBUG, "SMBUS: rangeley_smbus_read_resouce\n");

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