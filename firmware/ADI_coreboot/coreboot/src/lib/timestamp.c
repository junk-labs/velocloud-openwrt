/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 * Copyright (C) 2014 Sage Electronic Engineering, LLC.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#include <stddef.h>
#include <stdint.h>
#include <console/console.h>
#include <cbmem.h>
#include <timestamp.h>
#include <arch/early_variables.h>
#include <smp/node.h>
#include <pc80/mc146818rtc.h>

#define MAX_TIMESTAMPS 30
#ifdef CONFIG_TIMESTAMP_PRINT_LEVEL
#define TIMESTAMP_PRINT_LEVEL CONFIG_TIMESTAMP_PRINT_LEVEL
#else
#define TIMESTAMP_PRINT_LEVEL BIOS_SPEW
#endif

static struct timestamp_table* ts_table_p CAR_GLOBAL = NULL;
static tsc_t ts_basetime CAR_GLOBAL = { .lo = 0, .hi =0 };

static void timestamp_stash(enum timestamp_id id, tsc_t ts_time);

static const char * get_tsc_id_name(enum timestamp_id id)
{
	const char *name;
	switch (id) {
	case TS_BASETIME:
		name = "BASETIME         ";
		break;
	case TS_START_ROMSTAGE:
		name = "START_ROMSTAGE   ";
		break;
	case TS_BEFORE_INITRAM:
		name = "BEFORE_INITRAM   ";
		break;
	case TS_AFTER_INITRAM:
		name = "AFTER_INITRAM    ";
		break;
	case TS_END_ROMSTAGE:
		name = "END_ROMSTAGE     ";
		break;
	case TS_START_VBOOT:
		name = "START_VBOOT      ";
		break;
	case TS_END_VBOOT:
		name = "END_VBOOT        ";
		break;
	case TS_START_COPYRAM:
		name = "START_COPYRAM    ";
		break;
	case TS_END_COPYRAM:
		name = "END_COPYRAM      ";
		break;
	case TS_START_RAMSTAGE:
		name = "START_RAMSTAGE   ";
		break;
	case TS_BEFORE_CAR_INIT:
		name = "BEFORE_CAR_INIT  ";
		break;
	case TS_AFTER_CAR_INIT:
		name = "AFTER_CAR_INIT   ";
		break;
	case TS_DEVICE_ENUMERATE:
		name = "DEVICE_ENUMERATE ";
		break;
	case TS_FSP_BEFORE_ENUMERATE:
		name = "FSP_BEFORE_POST_ENUMERATE ";
		break;
	case TS_FSP_AFTER_ENUMERATE:
		name = "FSP_AFTER_POST_ENUMERATE ";
		break;
	case TS_DEVICE_CONFIGURE:
		name = "DEVICE_CONFIGURE ";
		break;
	case TS_DEVICE_ENABLE:
		name = "DEVICE_ENABLE    ";
		break;
	case TS_DEVICE_INITIALIZE:
		name = "DEVICE_INITIALIZE";
		break;
	case TS_DEVICE_DONE:
		name = "DEVICE_DONE      ";
		break;
	case TS_CBMEM_POST:
		name = "CBMEM_POST       ";
		break;
	case TS_WRITE_TABLES:
		name = "WRITE_TABLES     ";
		break;
	case TS_FSP_BEFORE_FINALIZE:
		name = "FSP_BEFORE_FINALIZE ";
		break;
	case TS_FSP_AFTER_FINALIZE:
		name = "FSP_AFTER_FINALIZE ";
		break;
	case TS_LOAD_PAYLOAD:
		name = "LOAD_PAYLOAD     ";
		break;
	case TS_ACPI_WAKE_JUMP:
		name = "ACPI_WAKE_JUMP   ";
		break;
	case TS_SELFBOOT_JUMP:
		name = "SELFBOOT_JUMP    ";
		break;
	default:
		name = "Unknown TSC ID   ";
		break;
	}
	return name;
}


static uint64_t tsc_to_uint64(tsc_t tstamp)
{
	return (((uint64_t)tstamp.hi) << 32) + tstamp.lo;
}

static void timestamp_real_init(tsc_t base)
{
	struct timestamp_table* tst;

	tst = cbmem_add(CBMEM_ID_TIMESTAMP,
			sizeof(struct timestamp_table) +
			MAX_TIMESTAMPS * sizeof(struct timestamp_entry));

	if (!tst) {
		printk(BIOS_ERR, "ERROR: failed to allocate timestamp table\n");
		return;
	}

	tst->base_time = tsc_to_uint64(base);
	tst->max_entries = MAX_TIMESTAMPS;
	tst->num_entries = 0;

	car_set_var(ts_table_p, tst);
	/* skip printing the initial 0xffffffff_ffffffff value */
	if ((base.hi + 1) || (base.hi + 1)) {
		printk(TIMESTAMP_PRINT_LEVEL, "TSC: %s : 0x%08lx_%08lx\n",
		       get_tsc_id_name(TS_BASETIME),
		       (unsigned long)base.hi,(unsigned long)base.lo );
	}
}

void timestamp_add(enum timestamp_id id, tsc_t ts_time)
{
	struct timestamp_entry *tse;
	struct timestamp_table *ts_table = NULL;

	if (!boot_cpu())
		return;

	ts_table = car_get_var(ts_table_p);
	if (!ts_table) {
		timestamp_stash(id, ts_time);
		return;
	}
	if (ts_table->num_entries == ts_table->max_entries)
		return;

	tse = &ts_table->entries[ts_table->num_entries++];
	tse->entry_id = id;
	tse->entry_stamp = tsc_to_uint64(ts_time) - ts_table->base_time;

	timestamp_show_entry(ts_table->num_entries - 1);
}

void timestamp_add_now(enum timestamp_id id)
{
	timestamp_add(id, rdtsc());
}

#define MAX_TIMESTAMP_CACHE 8
struct timestamp_cache {
	enum timestamp_id id;
	tsc_t time;
} timestamp_cache[MAX_TIMESTAMP_CACHE] CAR_GLOBAL;

static int timestamp_entries CAR_GLOBAL = 0;

/**
 * timestamp_stash() allows to temporarily cache timestamps.
 * This is needed when timestamping before the CBMEM area
 * is initialized. The function timestamp_do_sync() is used to
 * write the timestamps to the CBMEM area and this is done as
 * part of CAR migration for romstage, and in ramstage main().
 */

static void timestamp_stash(enum timestamp_id id, tsc_t ts_time)
{
	struct timestamp_cache *ts_cache = car_get_var(timestamp_cache);
	int ts_entries = car_get_var(timestamp_entries);

	if (ts_entries >= MAX_TIMESTAMP_CACHE) {
		printk(BIOS_ERR, "ERROR: failed to add timestamp to cache\n");
		return;
	}
	ts_cache[ts_entries].id = id;
	ts_cache[ts_entries].time = ts_time;
	car_set_var(timestamp_entries, ++ts_entries);
}

/**
 * /brief displays timestamp by ID
 * @param id timestamp ID value
 */
void timestamp_show_id(enum timestamp_id id)
{
	struct timestamp_entry *tse;
	struct timestamp_table *ts_table = NULL;
	uint8_t i;

	ts_table = car_get_var(ts_table_p);
	if (!ts_table) {
		return;
	}

	if (id == 0){
		if (ts_table->base_time > 0xffffffff) {
				return;
		}

		printk(TIMESTAMP_PRINT_LEVEL, "TSC: %s : 0x%08lx_%08lx\n",
		       get_tsc_id_name(id),
		       (unsigned long)(ts_table->base_time >> 32),
		       (unsigned long)ts_table->base_time);
	} else {
		for (i = 0; i < ts_table->num_entries; i++) {
			tse = &ts_table->entries[i];
			if (tse->entry_id == id) {
				printk(TIMESTAMP_PRINT_LEVEL, "TSC: %s : 0x%08lx_%08lx\n",
				       get_tsc_id_name(id),
				       (unsigned long)(tse->entry_stamp >> 32),
				       (unsigned long)tse->entry_stamp);
			}
		}
	}
}

/**
 * /brief displays timestamp by entry number (0 based)
 * @param entry - entry num in the timestamp table
 */
void timestamp_show_entry(uint32_t entry)
{
	struct timestamp_entry *tse;
	struct timestamp_table *ts_table = NULL;

	ts_table = car_get_var(ts_table_p);
	if (!ts_table) {
		return;
	}

	if (entry < ts_table->num_entries) {
		tse = &ts_table->entries[entry];
		printk(TIMESTAMP_PRINT_LEVEL, "TSC: %s : 0x%08lx_%08lx\n",
		       get_tsc_id_name(tse->entry_id),
		       (unsigned long)(tse->entry_stamp >> 32),
		       (unsigned long)tse->entry_stamp);
	}
}

static void timestamp_do_sync(void)
{
	struct timestamp_cache *ts_cache = car_get_var(timestamp_cache);
	int ts_entries = car_get_var(timestamp_entries);

	int i;
	for (i = 0; i < ts_entries; i++)
		timestamp_add(ts_cache[i].id, ts_cache[i].time);
	car_set_var(timestamp_entries, 0);
}

void timestamp_init(tsc_t base)
{
	if (!boot_cpu())
		return;

#ifdef __PRE_RAM__
	/* Copy of basetime, it is too early for CBMEM. */
	car_set_var(ts_basetime, base);
#else
	struct timestamp_table* tst;

	/* Locate and use an already existing table. */
	tst = cbmem_find(CBMEM_ID_TIMESTAMP);
	if (tst) {
		car_set_var(ts_table_p, tst);
		return;
	}

	/* Copy of basetime, may be too early for CBMEM. */
	car_set_var(ts_basetime, base);
	timestamp_real_init(base);
#endif
}

void timestamp_reinit(void)
{
	if (!boot_cpu())
		return;

#ifdef __PRE_RAM__
	timestamp_real_init(car_get_var(ts_basetime));
#else
	if (!car_get_var(ts_table_p))
		timestamp_init(car_get_var(ts_basetime));
#endif
	if (car_get_var(ts_table_p))
		timestamp_do_sync();
}

/* Call timestamp_reinit at CAR migration time. */
CAR_MIGRATE(timestamp_reinit)

#if IS_ENABLED(CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS)
/**
 * get_timestamp_from_cmos
 * @param start_cmos_addr low cmos address to get the 8 bytes of the tsc from
 * @param time - pointer to tsc structure to put the time value into

 */
void get_timestamp_from_cmos(uint8_t start_cmos_addr, tsc_t *time)
{
	int8_t i;
	uint32_t val = 0;

	for (i = SIZE_OF_TSC - 1; i > 0; i--) {
		val <<= 8;
		val |= cmos_read(start_cmos_addr + i);

		if (i == 4) {
			time->hi = val;
			val = 0;
		}
	}
	time->lo = val;
}

/**
 * save_timestamp_to_cmos
 * @param start_cmos_addr - low cmos address to save the 8 bytes of the tsc to
 * @param time - TSC value to write out
 */
void save_timestamp_to_cmos(uint8_t start_cmos_addr, tsc_t time)
{
	uint8_t i;
	uint32_t val = time.lo;

	printk(BIOS_DEBUG, "Saving TSC Value 0x%08lx_%08lx to address 0x%02x\n",
			(unsigned long )(time.hi), (unsigned long )(time.lo),
			start_cmos_addr);

	for (i = 0; i < SIZE_OF_TSC; i++) {

		cmos_write(val & 0xff, start_cmos_addr + i);
		val >>= 8;

		if (i == 3)
			val = time.hi;
	}
}
#endif // CONFIG_SAVE_EARLY_TIMESTAMPS_TO_CMOS
