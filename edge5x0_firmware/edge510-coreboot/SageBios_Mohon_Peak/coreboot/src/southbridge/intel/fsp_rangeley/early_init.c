/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdint.h>
#include <stdlib.h>
#include <console/console.h>
#include <arch/io.h>
#include <device/pci_def.h>
#include <device/device.h>
#include <elog.h>
#include <pc80/mc146818rtc.h>
#include <build.h>
#include "chip.h"
#include "pci_devs.h"
#include "soc.h"

typedef struct southbridge_intel_fsp_rangeley_config config_t;

static void limit_sata_speed(uint32_t sata_dev, uint32_t sata_base, uint8_t gen)
{
	uint32_t reg32;
	uint16_t reg16;
	uint16_t original_map_value;
	uint16_t original_cmd_value;
	uint32_t original_abar_value;

	/* Set the controller mode to ahci mode so abar is valid */
	original_map_value = pci_read_config16(sata_dev, SATA_MAP);
	reg16 = original_map_value & SATA_MODE_SELECT_MASK;
	reg16 |= (SATA_MODE_AHCI << SATA_MODE_SELECT_SHIFT);
	pci_write_config16(sata_dev, SATA_MAP, reg16);

	/* Set the base address */
	original_abar_value = pci_read_config32(sata_dev, SATA_ABAR_REG);
	pci_write_config32(sata_dev, SATA_ABAR_REG, sata_base);

	/* Enable SATA AHCI memory bars */
	original_cmd_value = pci_read_config16(sata_dev, SATA_CMD_REG);
	reg16 = original_cmd_value | SATA_MEM_SPACE_ENABLE;
	pci_write_config16(sata_dev, SATA_CMD_REG, reg16);

	pci_write_config32(sata_dev, SATA_ABAR_REG, sata_base);

	/* Set SATA device's speed limit */
	reg32 = read32(sata_base + GHC_CAP_REG);
	reg32 &= GHC_CAP_ISS_MASK;
	reg32 |= (gen << GHC_CAP_ISS_SHIFT);
	write32(sata_base + GHC_CAP_REG, reg32);

	printk(BIOS_SPEW,
		"SATA: GHC_CAP_REG (0x%08lx)set to 0x%08lx\n",
		(unsigned long)(sata_base + GHC_CAP_REG),
		(unsigned long)read32(sata_base + GHC_CAP_REG));

	/* Disable SATA BARs again */
	pci_write_config16(sata_dev, SATA_CMD_REG, original_cmd_value);
	pci_write_config32(sata_dev, SATA_ABAR_REG, original_abar_value);
	pci_write_config16(sata_dev, SATA_MAP, original_map_value);
}

static void rangeley_print_stepping(void)
{
	char const *stepping_str;

	u8 soc_stepping = pci_read_config8(SOC_BDF, PCI_REVISION_ID);

	switch (soc_stepping) {

	case 0:
		stepping_str = "A0";
		break;
	case 1:
		stepping_str = "A1";
		break;
	case 2:
		stepping_str = "B0";
		break;
	case 3:
		stepping_str = "C0";
		break;
	default:
		stepping_str = "unknown";
	}

	printk(BIOS_INFO, "Detected %s stepping SOC\n", stepping_str);
}

static void rangeley_setup_bars(void)
{
	ROMSTAGE_CONST struct device *dev;
	ROMSTAGE_CONST config_t *config;
	dev = dev_find_slot(0, SATA2_DEV_FUNC);
	config = dev->chip_info;

	rangeley_print_stepping();
	/* Setting up Southbridge. */


	printk(BIOS_DEBUG, "Setting up static southbridge registers...");
	pci_write_config32(LPC_BDF, RCBA, DEFAULT_RCBA | RCBA_ENABLE);
	pci_write_config32(LPC_BDF, ABASE, DEFAULT_ABASE | SET_BAR_ENABLE);
	pci_write_config32(LPC_BDF, PBASE, DEFAULT_PBASE | SET_BAR_ENABLE);
	printk(BIOS_DEBUG, " done.\n");

	printk(BIOS_DEBUG, "Disabling Watchdog timer...");
	/* Disable the watchdog reboot and turn off the watchdog timer */
	write8(DEFAULT_PBASE + PMC_CFG, read8(DEFAULT_PBASE + PMC_CFG) |
		 NO_REBOOT);	// disable reboot on timer trigger
	outw(DEFAULT_ABASE + TCO1_CNT, inw(DEFAULT_ABASE + TCO1_CNT) |
		TCO_TMR_HALT);	// disable watchdog timer

#if CONFIG_ELOG_BOOT_COUNT
	/* Increment Boot Counter for non-S3 resume */
	if ((inw(DEFAULT_ABASE + PM1_STS) & WAK_STS) &&
	    ((inl(DEFAULT_ABASE + PM1_CNT) >> 10) & 7) != SLP_TYP_S3)
		boot_count_increment();
#endif

	printk(BIOS_DEBUG, " done.\n");

#if CONFIG_ELOG_BOOT_COUNT
	/* Increment Boot Counter except when resuming from S3 */
	if ((inw(DEFAULT_ABASE + PM1_STS) & WAK_STS) &&
	    ((inl(DEFAULT_ABASE + PM1_CNT) >> 10) & 7) == SLP_TYP_S3)
		return;
	boot_count_increment();
#endif
	/*
	 * Set SATA controller max speed.
	 * These values are one-time writable,
	 * so this needs to be done before the FSP writes them.
	 */
	if (config->sata_ahci && config->sata_max_speed) {
		printk(BIOS_DEBUG,
		"Setting SATA controllers max speed to Gen %d speeds.\n",
		config->sata_max_speed);

		limit_sata_speed(SATA2_BDF,DEFAULT_SATA2_ABASE,
			config->sata_max_speed);
		limit_sata_speed(SATA3_BDF,DEFAULT_SATA3_ABASE,
			config->sata_max_speed);
	}

}

static void reset_rtc(void)
{
	uint32_t pbase = pci_read_config32(LPC_BDF, PBASE) &
		PCI_BASE_ADDRESS_MEM_ADDR_MASK;
	uint32_t gen_pmcon1 = read32(pbase + GEN_PMCON1);
	int rtc_failed = !!(gen_pmcon1 & RPS);

	if (rtc_failed) {
		printk(BIOS_DEBUG,
			"RTC Failure detected.  Resetting Date to %x/%x/%x%x\n",
			COREBOOT_BUILD_MONTH_BCD,
			COREBOOT_BUILD_DAY_BCD,
			COREBOOT_BUILD_CENTURY_BCD,
			COREBOOT_BUILD_YEAR_BCD);

		/* Clear the power failure flag */
		write32(DEFAULT_PBASE + GEN_PMCON1, gen_pmcon1 & ~RPS);
	}

	rtc_init(rtc_failed);
}

void rangeley_sb_early_initialization(void)
{
	/* Setup all BARs required for early PCIe and raminit */
	rangeley_setup_bars();

	reset_rtc();
}
