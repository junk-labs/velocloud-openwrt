/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2010 Marc Bertens <mbertens@xs4all.nl>
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

#include <device/device.h>
#include <device/pci_def.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <console/console.h>

static void dec_21143_enable(device_t dev)
{
	printk(BIOS_DEBUG, "Initializing DECchip 21143\n");

}

static struct device_operations dec_21143_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = dec_21143_enable,
	.scan_bus         = 0,
};

static const struct pci_driver dec_21143_driver __pci_driver = {
	.ops    = &dec_21143_ops,
	.vendor = PCI_VENDOR_ID_DEC,
	.device = PCI_DEVICE_ID_DEC_21142, // FIXME wrong ID?
};
