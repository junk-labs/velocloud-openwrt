/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Sven Schnelle <svens@stackframe.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <smbios.h>
#include <console/console.h>
#include <build.h>
#include <device/device.h>
#include <arch/cpu.h>
#include <cpu/x86/name.h>
#include <cbfs_core.h>
#include <arch/byteorder.h>
#include <elog.h>
#if IS_ENABLED(CONFIG_CHROMEOS)
#include <vendorcode/google/chromeos/gnvs.h>
#endif

#include <spi-generic.h>
#include <spi_flash.h>

static u8 smbios_checksum(u8 *p, u32 length)
{
	u8 ret = 0;
	while (length--)
		ret += *p++;
	return -ret;
}


int smbios_add_string(char *start, const char *str)
{
	int i = 1;
	char *p = start;

	/*
	 * If the string is empty, return 0
	 * as required for empty strings
	 * (Version 2.7.1 of the SMBIOS
	 * specification, Section 6.1.3)
	 */
	if (*str == '\0') {
		return 0;
	}

	for(;;) {
		if (!*p) {
			strcpy(p, str);
			p += strlen(str);
			*p++ = '\0';
			*p++ = '\0';
			return i;
		}

		if (!strcmp(p, str))
			return i;

		p += strlen(p)+1;
		i++;
	}
}

int smbios_string_table_len(char *start)
{
	char *p = start;
	int i, len = 0;

	while(*p) {
		i = strlen(p) + 1;
		p += i;
		len += i;
	}
	return len + 1;
}

static int smbios_cpu_vendor(char *start)
{
	char tmp[13] = "Unknown";
	u32 *_tmp = (u32 *)tmp;
	struct cpuid_result res;

	if (cpu_have_cpuid()) {
		res = cpuid(0);
		_tmp[0] = res.ebx;
		_tmp[1] = res.edx;
		_tmp[2] = res.ecx;
		tmp[12] = '\0';
	}

	return smbios_add_string(start, tmp);
}

static int smbios_processor_name(char *start)
{
	char tmp[49] = "Unknown Processor Name";
	u32  *_tmp = (u32 *)tmp;
	struct cpuid_result res;
	int i;

	if (cpu_have_cpuid()) {
		res = cpuid(0x80000000);
		if (res.eax >= 0x80000004) {
			for (i = 0; i < 3; i++) {
				res = cpuid(0x80000002 + i);
				_tmp[i * 4 + 0] = res.eax;
				_tmp[i * 4 + 1] = res.ebx;
				_tmp[i * 4 + 2] = res.ecx;
				_tmp[i * 4 + 3] = res.edx;
			}
			tmp[48] = 0;
		}
	}
	return smbios_add_string(start, tmp);
}

static int smbios_write_type0(unsigned long *current, int handle)
{
	struct smbios_type0 *t = (struct smbios_type0 *)*current;
	int len = sizeof(struct smbios_type0);

	memset(t, 0, sizeof(struct smbios_type0));
	t->type = SMBIOS_BIOS_INFORMATION;
	t->handle = handle;
	t->length = len - 2;

	t->vendor = smbios_add_string(t->eos, "coreboot");
#if !IS_ENABLED(CONFIG_CHROMEOS)
	t->bios_release_date = smbios_add_string(t->eos, COREBOOT_DMI_DATE);

	if (strlen(CONFIG_LOCALVERSION))
		t->bios_version = smbios_add_string(t->eos, CONFIG_LOCALVERSION);
	else
		t->bios_version = smbios_add_string(t->eos, COREBOOT_VERSION);
#else
#define SPACES \
	"                                                                  "
	t->bios_release_date = smbios_add_string(t->eos, COREBOOT_DMI_DATE);
	u32 version_offset = (u32)smbios_string_table_len(t->eos);
	t->bios_version = smbios_add_string(t->eos, SPACES);
	/* SMBIOS offsets start at 1 rather than 0 */
	vboot_data->vbt10 = (u32)t->eos + (version_offset - 1);
#endif

	{
		const struct cbfs_header *header;
		u32 romsize = CONFIG_ROM_SIZE;
		header = cbfs_get_header(CBFS_DEFAULT_MEDIA);
		if (header != CBFS_HEADER_INVALID_ADDRESS)
			romsize = ntohl(header->romsize);
		t->bios_rom_size = (romsize / 65535) - 1;
	}

	t->system_bios_major_release = 4;
	t->bios_characteristics =
		BIOS_CHARACTERISTICS_PCI_SUPPORTED |
#if CONFIG_CARDBUS_PLUGIN_SUPPORT
		BIOS_CHARACTERISTICS_PC_CARD |
#endif
		BIOS_CHARACTERISTICS_SELECTABLE_BOOT |
		BIOS_CHARACTERISTICS_UPGRADEABLE;

#if CONFIG_GENERATE_ACPI_TABLES
	t->bios_characteristics_ext1 = BIOS_EXT1_CHARACTERISTICS_ACPI;
#endif
	t->bios_characteristics_ext2 = BIOS_EXT2_CHARACTERISTICS_TARGET;
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

#define ADI_OEM_SYS_SN_SIZE 13
#define ADI_OEM_UUID_SIZE 16
#define ADI_OEM_BOARD_SN_SIZE 14
#define ADI_OEM_PRODUCT_NAME_SIZE 10
#define ADI_OEM_BOARD_VERSION_SIZE 4
#define ADI_OEM_DMI_SIZE (ADI_OEM_SYS_SN_SIZE + ADI_OEM_UUID_SIZE + ADI_OEM_BOARD_SN_SIZE + ADI_OEM_PRODUCT_NAME_SIZE + ADI_OEM_BOARD_VERSION_SIZE)

static char oem_dmi[ADI_OEM_DMI_SIZE  +1 ]={0};
static char sys_sn[ADI_OEM_SYS_SN_SIZE +1] = {0};	
const char *__attribute__((weak)) smbios_system_serial_number(void)
{
	struct spi_flash *flash;
	int i;
        
	spi_init();

	printk(BIOS_DEBUG, "Probing SPI flash to get ready for SMBIOS update ");
	flash  = spi_flash_probe(0, 0,1000000, SPI_MODE_3);

	if (!flash) {
		printk(BIOS_DEBUG, "Could not find SPI device\n");
		return NULL;
	}

	/* read dmi area in OEM region, offset 0xf00. ADI_OEM_DMI_SIZE bytes total */
	flash->read(flash,0xf00,ADI_OEM_DMI_SIZE*sizeof(char),oem_dmi);
	/* copy our system serial number from dmi area */
	for (i=0; i < ADI_OEM_SYS_SN_SIZE ; i++)
	{
		sys_sn[i] = oem_dmi[i];
	}

	printk(BIOS_DEBUG, "SN:%s\n",sys_sn);

	return sys_sn;

	/* return CONFIG_MAINBOARD_SERIAL_NUMBER;*/
}


static char board_sn[ADI_OEM_BOARD_SN_SIZE+1] = {0};	
const char *__attribute__((weak)) smbios_mainboard_serial_number(void)
{
	int i;
        	
	/* copy our system serial number from dmi area */
	for (i=0; i< ADI_OEM_BOARD_SN_SIZE ; i++)
	{
		board_sn[i] = oem_dmi[i + ADI_OEM_SYS_SN_SIZE + ADI_OEM_UUID_SIZE];
	}

	printk(BIOS_DEBUG, "BSN:%s\n",board_sn);

	return board_sn;

}

static char board_version[ADI_OEM_BOARD_VERSION_SIZE + 1] = {0};
const char *__attribute__((weak)) smbios_mainboard_version(void)
{
	int i;
        	
	/* copy board version from dmi area */
	for (i=0; i< ADI_OEM_BOARD_VERSION_SIZE; i++)
	{
		board_version[i] = oem_dmi[i + ADI_OEM_SYS_SN_SIZE + ADI_OEM_UUID_SIZE + ADI_OEM_BOARD_SN_SIZE + ADI_OEM_PRODUCT_NAME_SIZE ];
	}

	printk(BIOS_DEBUG, "Board Version:%s\n",board_version);

	return board_version; 
}

const char *__attribute__((weak)) smbios_mainboard_manufacturer(void)
{
	return CONFIG_MAINBOARD_SMBIOS_MANUFACTURER;
}

static char product_name[ADI_OEM_PRODUCT_NAME_SIZE + 1] = {0};
const char *__attribute__((weak)) smbios_mainboard_product_name(void)
{

	int i;
        	
	for (i=0; i< ADI_OEM_PRODUCT_NAME_SIZE; i++)
	{
		product_name[i] = oem_dmi[i + ADI_OEM_SYS_SN_SIZE + ADI_OEM_UUID_SIZE + ADI_OEM_BOARD_SN_SIZE];
	}

	printk(BIOS_DEBUG, "BSN:%s\n",board_sn);
	return product_name;
}

void __attribute__((weak)) smbios_mainboard_set_uuid(u8 *uuid)
{
	int i;

	/* UUID binary storage in SMBIOS, 
         * byte order:  first 3 fields little endia, last two fields, big endian
         The UUID {00112233-4455-6677-8899-AABBCCDDEEFF} would thus be represented as:
                33 22 11 00 55 44 77 66 88 99 AA BB CC DD EE FF.
	*/

	for (i=0; i< 4 ; i++)
	{
		uuid[i] = oem_dmi[3-i + ADI_OEM_SYS_SN_SIZE];
	}
	
	uuid[4] =  oem_dmi[5 + ADI_OEM_SYS_SN_SIZE];
	uuid[5] =  oem_dmi[4 + ADI_OEM_SYS_SN_SIZE];

	uuid[6] =  oem_dmi[7 + ADI_OEM_SYS_SN_SIZE];
	uuid[7] =  oem_dmi[6 + ADI_OEM_SYS_SN_SIZE];

	for (i=8; i< ADI_OEM_UUID_SIZE ; i++)
	{
		uuid[i] = oem_dmi[i + ADI_OEM_SYS_SN_SIZE];
	}

	printk(BIOS_DEBUG, "UUID:%s\n",uuid);



}

const char mainboard_sku[] = {"None Provided"};

const char *__attribute__((weak)) smbios_mainboard_sku(void)
{
	return mainboard_sku;
}

static int smbios_write_type1(unsigned long *current, int handle)
{
	struct smbios_type1 *t = (struct smbios_type1 *)*current;
	int len = sizeof(struct smbios_type1);

	memset(t, 0, sizeof(struct smbios_type1));
	t->type = SMBIOS_SYSTEM_INFORMATION;
	t->length = len - 2;
	t->handle = handle;
	t->serial_number = smbios_add_string(t->eos, smbios_system_serial_number());
	t->manufacturer = smbios_add_string(t->eos, smbios_mainboard_manufacturer());
	t->product_name = smbios_add_string(t->eos, smbios_mainboard_product_name());
	t->version = smbios_add_string(t->eos, smbios_mainboard_version());
	smbios_mainboard_set_uuid(t->uuid);
	t->sku = smbios_add_string(t->eos, smbios_mainboard_sku());
	t->family = smbios_add_string(t->eos, "None Provided");
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

static int smbios_write_type2(unsigned long *current, int handle)
{
	struct smbios_type2 *t = (struct smbios_type2 *)*current;
	int len = sizeof(struct smbios_type2);

	memset(t, 0, sizeof(struct smbios_type2));
	t->type = SMBIOS_BOARD_INFORMATION;
	t->handle = handle;
	t->length = len - 2;
	t->manufacturer = smbios_add_string(t->eos, smbios_mainboard_manufacturer());
	t->product_name = smbios_add_string(t->eos, smbios_mainboard_product_name());
	t->serial_number = smbios_add_string(t->eos, smbios_mainboard_serial_number());
	t->version = smbios_add_string(t->eos, smbios_mainboard_version());
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

static int smbios_write_type3(unsigned long *current, int handle)
{
	struct smbios_type3 *t = (struct smbios_type3 *)*current;
	int len = sizeof(struct smbios_type3);

	memset(t, 0, sizeof(struct smbios_type3));
	t->type = SMBIOS_SYSTEM_ENCLOSURE;
	t->length = len - 2;
	t->handle = handle;
	t->manufacturer = smbios_add_string(t->eos, CONFIG_MAINBOARD_SMBIOS_MANUFACTURER);
	t->_type = CONFIG_SMBIOS_SYSTEM_ENCLOSURE_TYPE;
	t->bootup_state = SMBIOS_STATE_SAFE;
	t->power_supply_state = SMBIOS_STATE_SAFE;
	t->thermal_state = SMBIOS_STATE_SAFE;
	t->security_status = SMBIOS_STATE_SAFE;
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

static int smbios_write_type4(unsigned long *current, int handle)
{
	struct cpuid_result res;
	struct smbios_type4 *t = (struct smbios_type4 *)*current;
	int len = sizeof(struct smbios_type4);

	/* Provide sane defaults even for CPU without CPUID */
	res.eax = res.edx = 0;
	res.ebx = 0x10000;

	if (cpu_have_cpuid()) {
		res = cpuid(1);
	}

	memset(t, 0, sizeof(struct smbios_type4));
	t->type = SMBIOS_PROCESSOR_INFORMATION;
	t->length = len - 2;
	t->handle = handle;
	t->socket_designation = smbios_add_string(t->eos, "P0");
	t->processor_type = 3;	/* System Processor */
	t->processor_family = (res.eax > 0) ? 0x0c : 0x6;
	t->processor_manufacturer = smbios_cpu_vendor(t->eos);
	t->processor_id[0] = res.eax;
	t->processor_id[1] = res.edx;
	t->processor_version = smbios_processor_name(t->eos);
	t->status = 0x41;	/* Socket Populated and CPU Enabled */
	t->processor_upgrade = 0x06;
#if IS_ENABLED(CONFIG_UDELAY_LAPIC_FIXED_FSB)
	t->external_clock = CONFIG_UDELAY_LAPIC_FIXED_FSB;
#else
	t->external_clock = 200;	/* TODO: Should come from apic_timer.c 'timer_fsb'*/
#endif
	t->max_speed = 1600;		/* TODO: Where can we get this generically? */
	t->current_speed = 1600;	/* TODO: Where can we get this generically? */
	t->core_count = (res.ebx >> 16) & 0xff;
	t->l1_cache_handle = 0xffff;
	t->l2_cache_handle = 0xffff;
	t->l3_cache_handle = 0xffff;
	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

int smbios_write_type11(unsigned long *current, int handle, const char **oem_strings, int count)
{
	struct smbios_type11 *t = (struct smbios_type11 *)*current;
	int i, len;

	memset(t, 0, sizeof *t);
	t->type = SMBIOS_OEM_STRINGS;
	t->handle = handle;
	t->length = len = sizeof *t - 2;

	for (i = 0; i < count; i++)
		t->count = smbios_add_string(t->eos, oem_strings[i]);

	len += smbios_string_table_len(t->eos);
	*current += len;
	return len;
}

static int smbios_write_type32(unsigned long *current, int handle)
{
	struct smbios_type32 *t = (struct smbios_type32 *)*current;
	int len = sizeof(struct smbios_type32);

	memset(t, 0, sizeof(struct smbios_type32));
	t->type = SMBIOS_SYSTEM_BOOT_INFORMATION;
	t->handle = handle;
	t->length = len - 2;
	*current += len;
	return len;
}

int smbios_write_type41(unsigned long *current, int *handle,
			const char *name, u8 instance, u16 segment,
			u8 bus, u8 device, u8 function)
{
	struct smbios_type41 *t = (struct smbios_type41 *)*current;
	int len = sizeof(struct smbios_type41);

	memset(t, 0, sizeof(struct smbios_type41));
	t->type = SMBIOS_ONBOARD_DEVICES_EXTENDED_INFORMATION;
	t->handle = *handle;
	t->length = len - 2;
	t->reference_designation = smbios_add_string(t->eos, name);
	t->device_type = SMBIOS_DEVICE_TYPE_OTHER;
	t->device_status = 1;
	t->device_type_instance = instance;
	t->segment_group_number = segment;
	t->bus_number = bus;
	t->device_number = device;
	t->function_number = function;

	len = t->length + smbios_string_table_len(t->eos);
	*current += len;
	*handle += 1;
	return len;
}

static int smbios_write_type127(unsigned long *current, int handle)
{
	struct smbios_type127 *t = (struct smbios_type127 *)*current;
	int len = sizeof(struct smbios_type127);

	memset(t, 0, sizeof(struct smbios_type127));
	t->type = SMBIOS_END_OF_TABLE;
	t->handle = handle;
	t->length = len - 2;
	*current += len;
	return len;
}

static int smbios_walk_device_tree(device_t tree, int *handle, unsigned long *current)
{
	device_t dev;
	int len = 0;

	for(dev = tree; dev; dev = dev->next) {
		printk(BIOS_INFO, "%s (%s)\n", dev_path(dev), dev_name(dev));

		if (dev->ops && dev->ops->get_smbios_data)
			len += dev->ops->get_smbios_data(dev, handle, current);
	}
	return len;
}

unsigned long smbios_write_tables(unsigned long current)
{
	struct smbios_entry *se;
	unsigned long tables;
	int len, handle = 0;

	current = ALIGN(current, 16);
	printk(BIOS_DEBUG, "%s: %08lx\n", __func__, current);

	se = (struct smbios_entry *)current;
	current += sizeof(struct smbios_entry);
	current = ALIGN(current, 16);

	tables = current;
	len = smbios_write_type0(&current, handle++);
	len += smbios_write_type1(&current, handle++);
	len += smbios_write_type2(&current, handle++);
	len += smbios_write_type3(&current, handle++);
	len += smbios_write_type4(&current, handle++);
#if CONFIG_ELOG
	len += elog_smbios_write_type15(&current, handle++);
#endif
	len += smbios_write_type32(&current, handle++);

	len += smbios_walk_device_tree(all_devices, &handle, &current);

	len += smbios_write_type127(&current, handle++);

	memset(se, 0, sizeof(struct smbios_entry));
	memcpy(se->anchor, "_SM_", 4);
	se->length = sizeof(struct smbios_entry);
	se->major_version = 2;
	se->minor_version = 7;
	se->max_struct_size = 24;
	se->struct_count = handle;
	memcpy(se->intermediate_anchor_string, "_DMI_", 5);

	se->struct_table_address = (u32)tables;
	se->struct_table_length = len;

	se->intermediate_checksum = smbios_checksum((u8 *)se + 0x10,
						    sizeof(struct smbios_entry) - 0x10);
	se->checksum = smbios_checksum((u8 *)se, sizeof(struct smbios_entry));
	return current;
}
