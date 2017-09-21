/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
 * Copyright (C) 2013 Sage Electronic Engineering, LLC.
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

/*
 * Datasheet:
 *  - Name: F81216AD
 *  - URL: http://www.fintek.com.tw/files/productfiles/F81216AD_V020P.pdf
 */
#ifndef SUPERIO_FINTEK_F81216AD_F81216AD_H
#define SUPERIO_FINTEK_F81216AD_F81216AD_H

/* Logical Device Numbers (LDN). */
#define	F81216AD_SP1	0x00	/* UART1 */
#define	F81216AD_SP2	0x01	/* UART2 */
#define	F81216AD_SP3	0x02	/* UART3 */
#define	F81216AD_SP4	0x03	/* UART4 */
#define	F81216AD_WDT	0x08	/* WATCHDOG */

#define F81216AD_48MHZ_CLK	0x01
#define F81216AD_24MHZ_CLK	0x00

#define F81216AD_CLK_SEL_REG	0x25

void f81216ad_enable_serial(device_t dev, u16 iobase);
void f81216ad_disable_wdt(device_t dev);

#endif
