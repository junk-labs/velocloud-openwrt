/*
 * This file is part of the coreboot project.
 *
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

/* Chip name - Change to match your chip */
#define PLATFORM_SIO_CHIP_NAME "Unknown Super I/O Chip"

/* Platform Lock & Unlock Mechanisms */
#define PLATFORM_LOCK_8787AA	0
#define PLATFORM_LOCK_7777AA	0
#define PLATFORM_LOCK_55AA		0
#define PLATFORM_USE_UNLOCK		PLATFORM_LOCK_8787AA | PLATFORM_LOCK_7777AA | PLATFORM_LOCK_55AA

/*
 * To enable a device, change the enable to 1 and put in the logical
 * device number of the device.  This should match what's in devicetree.cb.
 */
#define PLATFORM_ENABLE_PNP_PS2_KEYBOARD	0
#define PLATFORM_PS2_KBC_LDN				0
#define PLATFORM_ENABLE_PNP_PS2_MOUSE		0
#define PLATFORM_PS2_MOUSE_LDN				0
#define PLATFORM_ENABLE_PNP_SERIAL1			0
#define PLATFORM_SP1_LDN					0
#define PLATFORM_ENABLE_PNP_SERIAL2			0
#define PLATFORM_SP2_LDN					0
#define PLATFORM_ENABLE_PNP_SERIAL3			0
#define PLATFORM_SP3_LDN					0
#define PLATFORM_ENABLE_PNP_SERIAL4			0
#define PLATFORM_SP4_LDN					0
#define PLATFORM_ENABLE_PNP_SERIAL5			0
#define PLATFORM_SP5_LDN					0
#define PLATFORM_ENABLE_PNP_SERIAL6			0
#define PLATFORM_SP6_LDN					0
#define PLATFORM_ENABLE_PNP_PP1				0
#define PLATFORM_PP1_LDN					0
#define PLATFORM_ENABLE_PNP_PP2				0
#define PLATFORM_PP2_LDN					0
#define PLATFORM_ENABLE_PNP_FDC				0
#define PLATFORM_FDC_LDN					0
#define PLATFORM_ENABLE_PNP_HWM				0
#define PLATFORM_HWM_LDN					0
#define PLATFORM_ENABLE_PNP_GPIO1			0
#define PLATFORM_GPIO1_LDN					0
#define PLATFORM_ENABLE_PNP_SPI				0
#define PLATFORM_SPI_LDN					0

/*
 * If the uart clock needs to be set by setting a value, enter the value
 * and the register here.
 */
#define PLATFORM_INIT_UART_CLOCK_SPEED		0
#define SIO_CLK_SEL_REG						0
#define SIO_CLOCK_SPEED_VALUE				0

