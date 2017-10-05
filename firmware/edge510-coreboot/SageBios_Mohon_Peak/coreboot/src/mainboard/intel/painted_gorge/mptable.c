/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 * Copyright (C) 2013, Intel Corporation.
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

#include <console/console.h>
#include <arch/io.h>
#include <arch/ioapic.h>
#include <arch/smp/mpspec.h>
#include <device/pci.h>
#include <string.h>
#include <stdint.h>

// Generate MP-table IRQ numbers for PCI devices.
#define IO_APIC0 2

#define INT_A	0
#define INT_B	1
#define INT_C	2
#define INT_D	3
#define PCI_IRQ(dev, intLine)	(((dev)<<2) | intLine)

#define PIRQ_A 16
#define PIRQ_B 17
#define PIRQ_C 18
#define PIRQ_D 19
#define PIRQ_E 20
#define PIRQ_F 21
#define PIRQ_G 22
#define PIRQ_H 23

// ILB Base Address
#define ILB 0x50

#define ILB_IR0 0x20
#define ILB_IR1 0x22
#define ILB_IR2 0x24
#define ILB_IR3 0x26
#define ILB_IR4 0x28
#define ILB_IR5 0x2A
#define ILB_IR6 0x2C
#define ILB_IR7 0x2E

#define ILB_IR8 0x30
#define ILB_IR9 0x32
#define ILB_IR10 0x34
#define ILB_IR11 0x36
#define ILB_IR12 0x38
#define ILB_IR13 0x3A
#define ILB_IR14 0x3C
#define ILB_IR15 0x3E

#define ILB_IR16 0x40
#define ILB_IR17 0x42
#define ILB_IR18 0x44
#define ILB_IR19 0x46
#define ILB_IR20 0x48
#define ILB_IR21 0x4A
#define ILB_IR22 0x4C
#define ILB_IR23 0x4E

#define ILB_IR24 0x50
#define ILB_IR25 0x52
#define ILB_IR26 0x54
#define ILB_IR27 0x56
#define ILB_IR28 0x58
#define ILB_IR29 0x5A
#define ILB_IR30 0x5C
#define ILB_IR31 0x5E

// Assume ILB IP
#define DEFAULT_ILB_IP 0x43214321

static void *smp_write_config_table(void *v)
{
        struct mp_config_table *mc;
	unsigned char bus_chipset;
	unsigned char bus_pcie_a;
//	int bus_isa, i;
	int bus_isa;
	uint32_t pin, route;
	device_t dev;
	unsigned long ilb;

	dev = dev_find_slot(0, PCI_DEVFN(0x1F,0));
	ilb = pci_read_config32(dev, ILB) & 0xFFFFFE00;

        mc = (void *)(((char *)v) + SMP_FLOATING_TABLE_LEN);

	mptable_init(mc, LOCAL_APIC_ADDR);

        smp_write_processors(mc);

	/* Get bus numbers */
	bus_chipset = 0;

	/* PCIE slots */
	dev = dev_find_slot(0, PCI_DEVFN(0x1,0));
	if(dev) {
	  bus_pcie_a = pci_read_config8(dev, PCI_SECONDARY_BUS);
	} else {
		printk(BIOS_DEBUG, "ERROR - could not find PCIe Port #1 0:1.0, using defaults\n");
	  bus_pcie_a = 1;
	}

	mptable_write_buses(mc, NULL, &bus_isa);

	/*I/O APICs: APIC ID Version State Address*/
	smp_write_ioapic(mc, 2, 0x20, IO_APIC_ADDR);

	mptable_add_isa_interrupts(mc, bus_isa, IO_APIC0, 0);

	/* Internal PCI device */

	/* PCIE Port 1 - 4
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(1, INT_A), IO_APIC0, PIRQ_A);
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(2, INT_A), IO_APIC0, PIRQ_A);
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(3, INT_A), IO_APIC0, PIRQ_E);
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(4, INT_A), IO_APIC0, PIRQ_E);

	/* IQAT: device 11, function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x0b, INT_A), IO_APIC0, PIRQ_A);

	/* RCEC: device 15, function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x0f, INT_A), IO_APIC0, PIRQ_A);

	/*
	   SMBus #1 : device 19 function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x13, INT_A), IO_APIC0, PIRQ_A);

	/* GbE: device 20, function x(0 - 3)
	 */
	pin = (DEFAULT_ILB_IP >> (0 * 4)) & 0x0F;
	if(pin > 0) {
	  pin -= 1;
	  route = PIRQ_A + ((read16(ilb + ILB_IR20) >> (pin * 4)) & 0x07);
	  smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x14, pin), IO_APIC0, route);
	}

	/* EHCI: device 22, function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x16, INT_A), IO_APIC0, PIRQ_H);

	/* SATA2: device 23, function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x17, INT_A), IO_APIC0, PIRQ_D);

	/* SATA3: device 24, function 0
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x18, INT_A), IO_APIC0, PIRQ_D);

	/*
	   SMBus #0 : device 31 function 3
	 */
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_chipset, PCI_IRQ(0x1f, INT_B), IO_APIC0, PIRQ_C);

	/* PCIe 4x slot B
	 */
//	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_pcie_a, PCI_IRQ(0, INT_A), IO_APIC0, PIRQ_A);
//	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_pcie_a, PCI_IRQ(0, INT_B), IO_APIC0, PIRQ_B);
//	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_pcie_a, PCI_IRQ(0, INT_C), IO_APIC0, PIRQ_C);
//	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_pcie_a, PCI_IRQ(0, INT_D), IO_APIC0, PIRQ_D);

	/* There is no extension information... */

	/*Local Ints:	Type	Polarity    Trigger	Bus ID	 IRQ	APIC ID	PIN#*/
	mptable_lintsrc(mc, bus_isa);

	/* Compute the checksums */
	return mptable_finalize(mc);
}

unsigned long write_smp_table(unsigned long addr)
{
	void *v;
	v = smp_write_floating_table(addr, 0);
	return (unsigned long)smp_write_config_table(v);
}
