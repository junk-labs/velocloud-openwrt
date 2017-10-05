/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <pc80/mc146818rtc.h>
#include <pc80/isa-dma.h>
#include <pc80/i8259.h>
#include <arch/io.h>
#include <arch/ioapic.h>
#include <arch/acpi.h>
#include <cpu/cpu.h>
#include <elog.h>
#include "soc.h"
#include "irq.h"

#define NMI_OFF	0

#define ENABLE_ACPI_MODE_IN_COREBOOT	0
#define TEST_SMM_FLASH_LOCKDOWN		0

typedef struct southbridge_intel_fsp_rangeley_config config_t;

static void soc_enable_apic(struct device *dev)
{
	int i;
	u32 reg32;
	volatile u32 *ioapic_index = (volatile u32 *)(IO_APIC_ADDR);
	volatile u32 *ioapic_data = (volatile u32 *)(IO_APIC_ADDR + 0x10);
	u32 ilb_base = pci_read_config32(dev, IBASE) & ~0x0f;

	/*
	 * Enable ACPI I/O and power management.
	 * Set SCI IRQ to IRQ9
	 */
	write32(ilb_base + ILB_OIC, 0x100);  /* AEN */
	reg32 = read32(ilb_base + ILB_OIC);  /* Read back per BWG */
	write32(ilb_base + ILB_ACTL, 0);  /* ACTL bit 2:0 SCIS IRQ9 */

	*ioapic_index = 0;
	*ioapic_data = (1 << 25);

	/* affirm full set of redirection table entries ("write once") */
	*ioapic_index = 1;
	reg32 = *ioapic_data;
	*ioapic_index = 1;
	*ioapic_data = reg32;

	*ioapic_index = 0;
	reg32 = *ioapic_data;
	printk(BIOS_DEBUG, "Southbridge APIC ID = %x\n", (reg32 >> 24) & 0x0f);
	if (reg32 != (1 << 25))
		die("APIC Error\n");

	printk(BIOS_SPEW, "Dumping IOAPIC registers\n");
	for (i=0; i<3; i++) {
		*ioapic_index = i;
		printk(BIOS_SPEW, "  reg 0x%04x:", i);
		reg32 = *ioapic_data;
		printk(BIOS_SPEW, " 0x%08x\n", reg32);
	}

	*ioapic_index = 3; /* Select Boot Configuration register. */
	*ioapic_data = 1; /* Use Processor System Bus to deliver interrupts. */
}

static void soc_enable_serial_irqs(struct device *dev)
{
	u32 ibase;

	ibase = pci_read_config32(dev, IBASE) & ~0xF;

	/* Set packet length and toggle silent mode bit for one frame. */
	write8(ibase + ILB_SERIRQ_CNTL, (1 << 7));

#if !CONFIG_SERIRQ_CONTINUOUS_MODE
	write8(ibase + ILB_SERIRQ_CNTL, 0);
#endif
}

/*
 * Write PCI config space IRQ assignments.  PCI
 * devices have the INT_LINE (0x3C) and INT_PIN (0x3D)
 * registers which report interrupt routing information
 * to operating systems and drivers.  The INT_PIN
 * register is generally read only and reports which
 * interrupt pin A - D it uses.  The INT_LINE register
 * is configurable and reports which IRQ (generally the
 * PIC IRQs 1 - 15) it will use.  This needs to take
 * interrupt pin swizzling on devices that are downstream
 * on a PCI bridge into account.  This function will loop
 * through all enabled PCI devices and program the INT_LINE
 * register with the correct PIC IRQ number for the INT_PIN
 * that it uses.
 */
static void write_pci_config_irqs(void)
{
	device_t irq_dev;
	device_t targ_dev;
	int int_line = 0;
	int int_pin = 0;
	int targ_pin = 0;
	int devfn = 0;
	int targ_bus = 0;
	int targ_devfn = 0;
	int pirq = 0;
	int dev = 0;
	const struct rangeley_irq_route *ir = &global_rangeley_irq_route;

	if (ir == NULL) {
		printk(BIOS_WARNING, "Warning: Can't write PCI IRQ assignments because"
				" 'global_rangeley_irq_route' structure does not exist\n");
		return;
	}

	/*
	 * Loop through all enabled devices and program their
	 * INT_LINE, INT_PIN registers from values taken from
	 * the Interrupt Route registers in the ILB
	 */
	printk(BIOS_DEBUG, "PCI_CFG IRQ: Write PCI config space IRQ assignments\n");
	for(irq_dev = all_devices; irq_dev; irq_dev = irq_dev->next) {
		devfn = irq_dev->path.pci.devfn;

		/*
		 * Step 1: Get the INT_PIN and device structure to look for in the
		 * pirq_data table defined in the mainboard directory
		 */
		targ_dev = NULL;
		targ_pin = get_pci_irq_pins(irq_dev, &targ_dev);
		if (targ_dev == NULL || targ_pin < 1)
			continue;

		/* Get the original INT_PIN for record keeping */
		int_pin = pci_read_config8(irq_dev, PCI_INTERRUPT_PIN);

		targ_bus   = targ_dev->bus->secondary;
		targ_devfn = targ_dev->path.pci.devfn;
		dev = PCI_SLOT(targ_devfn);

		/* Find the PIRQ that is attached to the INT_PIN this device uses */
		pirq = (ir->pcidev[dev] >> ((targ_pin - 1) * 4)) & 0xF;

		/* Get the INT_LINE this device/function will use */
		int_line = ir->pic[pirq];

		/* Set the Interrupt Line register in PCI config space */
		pci_write_config8(irq_dev, PCI_INTERRUPT_LINE, int_line);

		/* Set this IRQ to level triggered since it is used by a PCI device */
		if (int_line != PIRQ_PIC_IRQDISABLE)
			i8259_configure_irq_trigger(int_line, IRQ_LEVEL_TRIGGERED);

		printk(BIOS_SPEW, "\tINT_PIN\t\t: %d (%s)\n",
						int_pin, pin_to_str(int_pin));
		if (targ_devfn != devfn)
			printk(BIOS_SPEW, "\tSwizzled to\t: %d (%s)\n",
							targ_pin, pin_to_str(targ_pin));
		printk(BIOS_SPEW, "\tPIRQ\t\t: %c\n"
						"\tINT_LINE\t: 0x%X (IRQ %d)\n",
						'A' + pirq, int_line, int_line);
	}
	printk(BIOS_DEBUG, "PCI_CFG IRQ: Finished writing PCI config space IRQ assignments\n");
}

static void soc_pirq_init(device_t dev)
{
	device_t irq_dev;
	int devn, i;
	int pirq;
	const u32 ibase = pci_read_config32(dev, IBASE) & ~0xF;
	const unsigned long pr_base = ibase + 0x08;
	const unsigned long ir_base = ibase + 0x20;
	const unsigned long actl = ibase;
	const struct rangeley_irq_route *ir = &global_rangeley_irq_route;

	/* Set up the PIRQ PIC routing based on static config. */
	printk(BIOS_SPEW, "Start writing IRQ assignments\n"
			"PIRQ\tA \tB \tC \tD \tE \tF \tG \tH\n"
			"IRQ ");
	for (devn = 0; devn < NUM_PIRQS; devn++) {
		write8(pr_base + devn*sizeof(ir->pic[devn]), ir->pic[devn]);
		printk(BIOS_SPEW, "\t%d", ir->pic[devn]);
	}
	printk(BIOS_SPEW, "\n\n");

	/* Set up the per device PIRQ routing based on static config. */
	printk(BIOS_SPEW, "\t\t\tPIRQ[A-H] routed to each INT_PIN[A-D]\n"
			"Dev\tINTA (IRQ)\tINTB (IRQ)\tINTC (IRQ)\tINTD (IRQ)\n");
	for (devn = 0; devn < NUM_IR_DEVS; devn++) {
		irq_dev = dev_find_slot(0,PCI_DEVFN(devn, 0));
		if (!(irq_dev && irq_dev->enabled)) {
			printk(BIOS_INFO, "%3d: Disabled, skip it\n", devn);
			continue;
		}
		write16(ir_base + devn*sizeof(ir->pcidev[devn]), ir->pcidev[devn]);

		printk(BIOS_SPEW, " %d: ", devn);
		for (i = 0; i < 4; i++) {
			pirq = (ir->pcidev[devn] >> (i * 4)) & 0xF;
			printk(BIOS_SPEW, "\t%-4c (%d)", 'A' + pirq, ir->pic[pirq]);
		}
		printk(BIOS_SPEW, "\n");
	}

	/* Route SCI to IRQ9 */
	write32(actl, (read32(actl) & ~SCIS_MASK) | SCIS_IRQ9);
	printk(BIOS_SPEW, "Finished writing IRQ assignments\n");

	/* Write IRQ assignments to PCI config space */
	write_pci_config_irqs();
}

static void soc_power_options(device_t dev)
{
	u8 reg8;
	u16 pmbase;
	u32 reg32;

	/* Get the chip configuration */
	config_t *config = dev->chip_info;

	int nmi_option;

	/* Which state do we want to goto after g3 (power restored)?
	 * 0 == S0 Full On
	 * 1 == S5 Soft Off
	 *
	 * If the option is not existent (Laptops), use Kconfig setting.
	 */
	/* TODO : Setup power state after power failure:
	const char *state;
	int pwr_on=CONFIG_MAINBOARD_POWER_ON_AFTER_POWER_FAIL;
	printk(BIOS_INFO, "Set power %s after power failure.\n", state);
	*/

	/* Set up NMI on errors. */
	reg8 = inb(0x61);
	reg8 &= 0x0f;		/* Higher Nibble must be 0 */
	reg8 &= ~(1 << 3);	/* IOCHK# NMI Enable */
	// reg8 &= ~(1 << 2);	/* PCI SERR# Enable */
	reg8 |= (1 << 2); /* PCI SERR# Disable for now */
	outb(reg8, 0x61);

	reg8 = inb(0x70);
	nmi_option = NMI_OFF;
	get_option(&nmi_option, "nmi");
	if (nmi_option) {
		printk(BIOS_INFO, "NMI sources enabled.\n");
		reg8 &= ~(1 << 7);	/* Set NMI. */
	} else {
		printk(BIOS_INFO, "NMI sources disabled.\n");
		reg8 |= ( 1 << 7);	/* Can't mask NMI from PCI-E and NMI_NOW */
	}
	outb(reg8, 0x70);

	/* Enable CPU_SLP# and Intel Speedstep, set SMI# rate down */
	/* TODO: Setup PMCON1 */
	/*reg16 = pci_read_config16(dev, GEN_PMCON_1);
	reg16 &= ~(3 << 0);	// SMI# rate 1 minute
	reg16 &= ~(1 << 10);	// Disable BIOS_PCI_EXP_EN for native PME
	*/
#if DEBUG_PERIODIC_SMIS
	/* Set DEBUG_PERIODIC_SMIS in soc.h to debug using
	 * periodic SMIs.
	 */
	/* reg16 |= (3 << 0); // Periodic SMI every 8s
	 */
#endif
	/* pci_write_config16(dev, GEN_PMCON_1, reg16);
	 */

	pmbase = pci_read_config16(dev, ABASE) & ~0xf;

	outl(config->gpe0_en, pmbase + GPE0_EN);
	outw(config->alt_gp_smi_en, pmbase + ALT_GP_SMI_EN);

	/* Set up power management block and determine sleep mode */
	reg32 = inl(pmbase + PM1_CNT); // PM1_CNT
	reg32 &= ~(7 << 10);	// SLP_TYP
	reg32 |= (1 << 0);	// SCI_EN
	outl(reg32, pmbase + PM1_CNT);
}

/* Disable the HPET, Clear the counter, and re-enable it. */
static void enable_hpet(void)
{
	write8(HPET_GCFG, 0x00);
	write32(HPET_MCV, 0x00000000);
	write32(HPET_MCV + 0x04, 0x00000000);
	write8(HPET_GCFG, 0x01);
}

#if CONFIG_HAVE_SMI_HANDLER
static void soc_lock_smm(struct device *dev)
{
#if TEST_SMM_FLASH_LOCKDOWN
	u8 reg8;
#endif

	if (acpi_slp_type != 3) {
#if ENABLE_ACPI_MODE_IN_COREBOOT
		printk(BIOS_DEBUG, "Enabling ACPI via APMC:\n");
		outb(0xe1, 0xb2); // Enable ACPI mode
		printk(BIOS_DEBUG, "done.\n");
#else
		printk(BIOS_DEBUG, "Disabling ACPI via APMC:\n");
		outb(0x1e, 0xb2); // Disable ACPI mode
		printk(BIOS_DEBUG, "done.\n");
#endif
	}

	/* Don't allow evil boot loaders, kernels, or
	 * userspace applications to deceive us:
	 */
	smm_lock();
}
#endif

static void soc_disable_smm_only_flashing(struct device *dev)
{
	u8 reg8;

	printk(BIOS_SPEW, "Enabling BIOS updates outside of SMM... ");
	reg8 = pci_read_config8(dev, 0xdc);	/* BIOS_CNTL */
	reg8 &= ~(1 << 5);
	pci_write_config8(dev, 0xdc, reg8);
}

static void soc_fixups(struct device *dev)
{
	/* TODO: Is there Rangeley fixups? */

}

static void soc_decode_init(struct device *dev)
{
	printk(BIOS_DEBUG, "soc_decode_init\n");
	/* TODO: Is there Rangeley LPC decode? */
}

static void lpc_init(struct device *dev)
{
	printk(BIOS_DEBUG, "soc: lpc_init\n");

	/* Set the value for PCI command register. */
	pci_write_config16(dev, PCI_COMMAND, 0x000f);

	/* IO APIC initialization. */
	soc_enable_apic(dev);

	soc_enable_serial_irqs(dev);

	/* Setup the PIRQ. */
	soc_pirq_init(dev);

	/* Setup power options. */
	soc_power_options(dev);

	/* Initialize power management */
	switch (soc_silicon_type()) {
	case SOC_TYPE_RANGELEY:
		break;
	default:
		printk(BIOS_DEBUG, "Unknown Chipset: 0x%04x\n", dev->device);
	}

	/* Initialize ISA DMA. */
	isa_dma_init();

	/* Initialize the High Precision Event Timers, if present. */
	enable_hpet();

	setup_i8259();

	/* The OS should do this? */
	/* Interrupt 9 should be level triggered (SCI) */
	i8259_configure_irq_trigger(9, 1);

	soc_disable_smm_only_flashing(dev);

#if CONFIG_HAVE_SMI_HANDLER
	soc_lock_smm(dev);
#endif

	soc_fixups(dev);
}

static void soc_lpc_read_resources(device_t dev)
{
	struct resource *res;
	config_t *config = dev->chip_info;
	u8 io_index = 0;

	/* Get the normal PCI resources of this device. */
	pci_dev_read_resources(dev);

	/* Add an extra subtractive resource for both memory and I/O. */
	res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
	res->base = 0;
	res->size = 0x1000;
	res->flags = IORESOURCE_IO | IORESOURCE_SUBTRACTIVE |
		     IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
	res->base = 0xff800000;
	res->size = 0x00800000; /* 8 MB for flash */
	res->flags = IORESOURCE_MEM | IORESOURCE_SUBTRACTIVE |
		     IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	res = new_resource(dev, 3); /* IOAPIC */
	res->base = IO_APIC_ADDR;
	res->size = 0x00001000;
	res->flags = IORESOURCE_MEM | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	/* Set SOC IO decode ranges if required.*/
	if ((config->gen1_dec & 0xFFFC) > 0x1000) {
		res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
		res->base = config->gen1_dec & 0xFFFC;
		res->size = (config->gen1_dec >> 16) & 0xFC;
		res->flags = IORESOURCE_IO | IORESOURCE_SUBTRACTIVE |
				 IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
	}

	if ((config->gen2_dec & 0xFFFC) > 0x1000) {
		res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
		res->base = config->gen2_dec & 0xFFFC;
		res->size = (config->gen2_dec >> 16) & 0xFC;
		res->flags = IORESOURCE_IO | IORESOURCE_SUBTRACTIVE |
				 IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
	}

	if ((config->gen3_dec & 0xFFFC) > 0x1000) {
		res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
		res->base = config->gen3_dec & 0xFFFC;
		res->size = (config->gen3_dec >> 16) & 0xFC;
		res->flags = IORESOURCE_IO | IORESOURCE_SUBTRACTIVE |
				 IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
	}

	if ((config->gen4_dec & 0xFFFC) > 0x1000) {
		res = new_resource(dev, IOINDEX_SUBTRACTIVE(io_index++, 0));
		res->base = config->gen4_dec & 0xFFFC;
		res->size = (config->gen4_dec >> 16) & 0xFC;
		res->flags = IORESOURCE_IO| IORESOURCE_SUBTRACTIVE |
				 IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
	}
}

static void soc_lpc_enable_resources(device_t dev)
{
	soc_decode_init(dev);
	return pci_dev_enable_resources(dev);
}

static void soc_lpc_enable(device_t dev)
{
	soc_enable(dev);
}

static void set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

static struct pci_operations pci_ops = {
	.set_subsystem = set_subsystem,
};

static struct device_operations device_ops = {
	.read_resources		= soc_lpc_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= soc_lpc_enable_resources,
	.init			= lpc_init,
	.enable			= soc_lpc_enable,
	.scan_bus		= scan_static_bus,
	.ops_pci		= &pci_ops,
};


/* IDs for LPC device of Intel 89xx Series Chipset */
static const unsigned short pci_device_ids[] = { 0x1F38, 0x1F39, 0x1F3A, 0x1F3B,
                                                 0 };

static const struct pci_driver soc_lpc __pci_driver = {
	.ops	 = &device_ops,
	.vendor	 = PCI_VENDOR_ID_INTEL,
	.devices = pci_device_ids,
};


