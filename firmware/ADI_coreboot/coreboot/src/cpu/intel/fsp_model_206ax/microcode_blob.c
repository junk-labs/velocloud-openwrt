/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 The ChromiumOS Authors.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

unsigned microcode[] = {
#if IS_ENABLED(CONFIG_SUPPORT_CPU_UCODE_IN_CBFS)

#if IS_ENABLED(CONFIG_CPU_INTEL_FSP_MODEL_206AX)
	#include <microcode-m12206a7_00000029.h>
#endif

#if IS_ENABLED(CONFIG_CPU_INTEL_FSP_MODEL_306AX)

#if !(IS_ENABLED(CONFIG_EXCLUDE_306A2_MICROCODE))
	#include <microcode-m12306a2_00000008.h>
#endif
#if !(IS_ENABLED(CONFIG_EXCLUDE_306A4_MICROCODE))
	#include <microcode-m12306a4_00000007.h>
#endif
#if !(IS_ENABLED(CONFIG_EXCLUDE_306A5_MICROCODE))
	#include <microcode-m12306a5_00000007.h>
#endif
#if !(IS_ENABLED(CONFIG_EXCLUDE_306A8_MICROCODE))
	#include <microcode-m12306a8_00000010.h>
#endif
#if !(IS_ENABLED(CONFIG_EXCLUDE_306A9_MICROCODE))
	#include <microcode-m12306a9_00000019.h>
#endif

#endif /* IS_ENABLED(CONFIG_CPU_INTEL_FSP_MODEL_306AX) */
#endif /* IS_ENABLED(CONFIG_SUPPORT_CPU_UCODE_IN_CBFS) */

};
