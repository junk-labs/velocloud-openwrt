/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
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
 *  - Name: F81216H
 */
#ifndef SUPERIO_FINTEK_F81216_F81216_H
#define SUPERIO_FINTEK_F81216_F81216_H

/* Logical Device Numbers (LDN). */
#define	F81216H_SP1	0x00	/* UART1 */
#define	F81216H_SP2	0x01	/* UART2 */
#define	F81216H_SP3	0x02	/* UART3 */
#define	F81216H_SP4	0x03	/* UART4 */

void f81216h_enable_serial(device_t dev, u16 iobase);

#endif
